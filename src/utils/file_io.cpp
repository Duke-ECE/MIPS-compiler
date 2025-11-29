/**
 * @file file_io.cpp
 * @brief 文件输入输出工具类实现
 */

#include "utils/file_io.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace utils {

// ============================================================================
// 文件读取
// ============================================================================

std::optional<std::string> FileIO::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

std::optional<std::vector<std::string>> FileIO::readLines(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    
    return lines;
}

// ============================================================================
// 文件写入
// ============================================================================

FileResult FileIO::writeFile(const std::string& path, const std::string& content) {
    // 确保目录存在
    std::filesystem::path filePath(path);
    if (filePath.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(filePath.parent_path(), ec);
        if (ec) {
            return FileResult("无法创建目录: " + filePath.parent_path().string());
        }
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        return FileResult("无法打开文件: " + path);
    }
    
    file << content;
    file.close();
    
    return FileResult(true);
}

FileResult FileIO::writeLines(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return FileResult("无法打开文件: " + path);
    }
    
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
    
    return FileResult(true);
}

// ============================================================================
// Intel HEX 格式输出
// ============================================================================

uint8_t FileIO::calculateHexChecksum(const std::vector<uint8_t>& bytes) {
    uint32_t sum = 0;
    for (uint8_t byte : bytes) {
        sum += byte;
    }
    // 取低8位的二补数
    return static_cast<uint8_t>((~(sum & 0xFF) + 1) & 0xFF);
}

std::vector<uint8_t> FileIO::wordToBytes(uint32_t word) {
    return {
        static_cast<uint8_t>((word >> 24) & 0xFF),
        static_cast<uint8_t>((word >> 16) & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF),
        static_cast<uint8_t>(word & 0xFF)
    };
}

std::string FileIO::generateHexRecord(uint8_t type, uint16_t address,
                                       const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    oss << ':';
    
    // 构建用于校验和计算的字节序列
    std::vector<uint8_t> bytes;
    
    // 字节数
    uint8_t byteCount = static_cast<uint8_t>(data.size());
    bytes.push_back(byteCount);
    oss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex 
        << static_cast<int>(byteCount);
    
    // 地址（高字节在前）
    uint8_t addrHigh = static_cast<uint8_t>((address >> 8) & 0xFF);
    uint8_t addrLow = static_cast<uint8_t>(address & 0xFF);
    bytes.push_back(addrHigh);
    bytes.push_back(addrLow);
    oss << std::setw(2) << static_cast<int>(addrHigh)
        << std::setw(2) << static_cast<int>(addrLow);
    
    // 记录类型
    bytes.push_back(type);
    oss << std::setw(2) << static_cast<int>(type);
    
    // 数据字节
    for (uint8_t byte : data) {
        bytes.push_back(byte);
        oss << std::setw(2) << static_cast<int>(byte);
    }
    
    // 计算并添加校验和
    uint8_t checksum = calculateHexChecksum(bytes);
    oss << std::setw(2) << static_cast<int>(checksum);
    
    return oss.str();
}

std::string FileIO::generateHex(const std::vector<uint32_t>& code, uint32_t startAddress, bool wordAddressing) {
    std::ostringstream oss;

    constexpr uint8_t RECORD_DATA = 0x00;
    constexpr uint8_t RECORD_EOF = 0x01;
    constexpr uint8_t RECORD_EXT_LINEAR_ADDR = 0x04;
    constexpr uint32_t IMEM_DEPTH = 4096;  // 固定 4KB = 4096 words

    // 输出固定 4096 行数据
    for (uint32_t i = 0; i < IMEM_DEPTH; i++) {
        uint32_t word = (i < code.size()) ? code[i] : 0x00000000;  // 不足的填充 0
        uint32_t byteAddress;
        if (wordAddressing) {
            // 数据段使用 word 地址（不乘 4）
            byteAddress = (startAddress + i);
        } else {
            // 指令内存使用字地址乘以 4 得到字节地址
            byteAddress = (startAddress + i) * 4;
        }

        // 如果地址超过 16 位，需要使用扩展地址记录
        if (byteAddress > 0xFFFF) {
            uint16_t upperAddr = static_cast<uint16_t>((byteAddress >> 16) & 0xFFFF);
            std::vector<uint8_t> extAddrData = {
                static_cast<uint8_t>((upperAddr >> 8) & 0xFF),
                static_cast<uint8_t>(upperAddr & 0xFF)
            };
            oss << generateHexRecord(RECORD_EXT_LINEAR_ADDR, 0x0000, extAddrData) << "\n";
        }

        // 生成数据记录
        std::vector<uint8_t> data = wordToBytes(word);
        uint16_t addr16 = static_cast<uint16_t>(byteAddress & 0xFFFF);
        oss << generateHexRecord(RECORD_DATA, addr16, data) << "\n";
    }

    // 添加 EOF 记录（第 4097 行）
    oss << generateHexRecord(RECORD_EOF, 0x0000, {}) << "\n";

    return oss.str();
}

FileResult FileIO::writeHex(const std::string& path,
                            const std::vector<uint32_t>& code,
                            uint32_t startAddress,
                            bool wordAddressing) {
    std::string content = generateHex(code, startAddress, wordAddressing);
    return writeFile(path, content);
}

// ============================================================================
// MIF 格式输出
// ============================================================================

std::string FileIO::generateMif(const std::vector<uint32_t>& code,
                                 uint32_t depth,
                                 uint32_t width) {
    // 自动计算深度
    if (depth == 0) {
        depth = static_cast<uint32_t>(code.size());
        // 向上取整到 2 的幂次
        uint32_t powerOf2 = 1;
        while (powerOf2 < depth) {
            powerOf2 *= 2;
        }
        depth = powerOf2;
    }
    
    std::ostringstream oss;
    
    // MIF 文件头
    oss << "-- Memory Initialization File (.mif)\n";
    oss << "-- Generated by Duke 550 Assembler\n";
    oss << "\n";
    oss << "DEPTH = " << depth << ";\n";
    oss << "WIDTH = " << width << ";\n";
    oss << "\n";
    oss << "ADDRESS_RADIX = HEX;\n";
    oss << "DATA_RADIX = HEX;\n";
    oss << "\n";
    oss << "CONTENT\n";
    oss << "BEGIN\n";
    
    // 写入数据
    for (size_t i = 0; i < code.size(); i++) {
        oss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << i
            << " : "
            << std::setw(8) << code[i] << ";\n";
    }
    
    // 填充剩余地址为 0
    if (code.size() < depth) {
        oss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << code.size()
            << ".."
            << std::setw(4) << (depth - 1)
            << " : 00000000;\n";
    }
    
    oss << "END;\n";
    
    return oss.str();
}

FileResult FileIO::writeMif(const std::string& path,
                            const std::vector<uint32_t>& code,
                            uint32_t depth,
                            uint32_t width) {
    std::string content = generateMif(code, depth, width);
    return writeFile(path, content);
}

// ============================================================================
// 路径工具
// ============================================================================

bool FileIO::exists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string FileIO::getExtension(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.extension().string();
}

std::string FileIO::getFileName(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.filename().string();
}

std::string FileIO::getBaseName(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.stem().string();
}

std::string FileIO::getDirectory(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.parent_path().string();
}

std::string FileIO::replaceExtension(const std::string& path, const std::string& newExt) {
    std::filesystem::path filePath(path);
    filePath.replace_extension(newExt);
    return filePath.string();
}

std::string FileIO::joinPath(const std::string& dir, const std::string& filename) {
    std::filesystem::path dirPath(dir);
    return (dirPath / filename).string();
}

} // namespace utils
