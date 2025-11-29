/**
 * @file hex_writer.hpp
 * @brief Intel HEX 格式文件写入器
 * 
 * Intel HEX 是一种 ASCII 文本格式，用于传输二进制数据到
 * EPROM、微控制器、FPGA 等可编程设备。
 * 
 * 记录格式: :LLAAAATT[DD...]CC
 * - LL: 数据字节数
 * - AAAA: 16位地址
 * - TT: 记录类型 (00=数据, 01=EOF, 04=扩展线性地址)
 * - DD: 数据字节
 * - CC: 校验和
 */

#ifndef HEX_WRITER_HPP
#define HEX_WRITER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace assembler {

/**
 * @brief Intel HEX 格式文件写入器
 */
class HexWriter {
public:
    // ==================== 记录类型常量 ====================
    static constexpr uint8_t RECORD_DATA = 0x00;           // 数据记录
    static constexpr uint8_t RECORD_EOF = 0x01;            // 文件结束记录
    static constexpr uint8_t RECORD_EXT_SEG_ADDR = 0x02;   // 扩展段地址记录
    static constexpr uint8_t RECORD_START_SEG_ADDR = 0x03; // 起始段地址记录
    static constexpr uint8_t RECORD_EXT_LINEAR_ADDR = 0x04;// 扩展线性地址记录
    static constexpr uint8_t RECORD_START_LINEAR_ADDR = 0x05; // 起始线性地址记录

    /**
     * @brief 将 32 位字序列写入 Intel HEX 格式文件
     * @param words 32位机器码序列
     * @param path 输出文件路径
     * @param startAddress 起始地址（字地址，默认0）
     * @return 是否写入成功
     */
    static bool writeFile(const std::vector<uint32_t>& words, 
                          const std::string& path,
                          uint32_t startAddress = 0);

    /**
     * @brief 生成 Intel HEX 格式字符串（不写入文件）
     * @param words 32位机器码序列
     * @param startAddress 起始地址（字地址，默认0）
     * @return Intel HEX 格式的完整字符串（默认 4096 行数据 + EOF）
     */
    static std::string generate(const std::vector<uint32_t>& words,
                                uint32_t startAddress = 0);

    /**
     * @brief 生成 Intel HEX 格式字符串（指定内存深度）
     * @param words 32位机器码序列
     * @param startAddress 起始地址（字地址）
     * @param depth 内存深度（总数据行数，如 4096 表示 4KB）
     * @return Intel HEX 格式的完整字符串（depth 行数据 + EOF）
     */
    static std::string generate(const std::vector<uint32_t>& words,
                                uint32_t startAddress,
                                uint32_t depth);

    /**
     * @brief 生成单条 Intel HEX 记录
     * @param type 记录类型
     * @param address 16位地址
     * @param data 数据字节
     * @return HEX 记录字符串（包含起始冒号）
     */
    static std::string generateRecord(uint8_t type, 
                                       uint16_t address, 
                                       const std::vector<uint8_t>& data);

    /**
     * @brief 生成数据记录
     * @param address 16位地址
     * @param data 数据字节
     * @return 数据记录字符串
     */
    static std::string generateDataRecord(uint16_t address,
                                           const std::vector<uint8_t>& data);

    /**
     * @brief 生成 EOF 记录
     * @return EOF 记录字符串 ":00000001FF"
     */
    static std::string generateEofRecord();

    /**
     * @brief 生成扩展线性地址记录
     * @param upperAddress 高16位地址
     * @return 扩展地址记录字符串
     */
    static std::string generateExtendedAddressRecord(uint16_t upperAddress);

    /**
     * @brief 计算 Intel HEX 校验和
     * @param bytes 字节序列（不包含校验和本身）
     * @return 校验和字节
     */
    static uint8_t calculateChecksum(const std::vector<uint8_t>& bytes);

    /**
     * @brief 将 32 位字拆分为字节（大端序）
     * @param word 32位字
     * @return 4个字节的向量（高字节在前）
     */
    static std::vector<uint8_t> wordToBytes(uint32_t word);

private:
    /**
     * @brief 将字节转换为两位十六进制字符串
     * @param byte 字节值
     * @return 两位大写十六进制字符串
     */
    static std::string byteToHex(uint8_t byte);
};

// ============================================================================
// 内联实现
// ============================================================================

inline std::string HexWriter::byteToHex(uint8_t byte) {
    std::ostringstream oss;
    oss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex 
        << static_cast<int>(byte);
    return oss.str();
}

inline uint8_t HexWriter::calculateChecksum(const std::vector<uint8_t>& bytes) {
    uint32_t sum = 0;
    for (uint8_t byte : bytes) {
        sum += byte;
    }
    // 取低8位的二补数
    return static_cast<uint8_t>((~(sum & 0xFF) + 1) & 0xFF);
}

inline std::vector<uint8_t> HexWriter::wordToBytes(uint32_t word) {
    return {
        static_cast<uint8_t>((word >> 24) & 0xFF),
        static_cast<uint8_t>((word >> 16) & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF),
        static_cast<uint8_t>(word & 0xFF)
    };
}

inline std::string HexWriter::generateEofRecord() {
    return ":00000001FF";
}

inline std::string HexWriter::generateExtendedAddressRecord(uint16_t upperAddress) {
    std::vector<uint8_t> data = {
        static_cast<uint8_t>((upperAddress >> 8) & 0xFF),
        static_cast<uint8_t>(upperAddress & 0xFF)
    };
    return generateRecord(RECORD_EXT_LINEAR_ADDR, 0x0000, data);
}

inline std::string HexWriter::generateDataRecord(uint16_t address,
                                                  const std::vector<uint8_t>& data) {
    return generateRecord(RECORD_DATA, address, data);
}

inline std::string HexWriter::generateRecord(uint8_t type, 
                                              uint16_t address,
                                              const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    oss << ':';
    
    // 构建用于校验和计算的字节序列
    std::vector<uint8_t> bytes;
    
    // 字节数
    uint8_t byteCount = static_cast<uint8_t>(data.size());
    bytes.push_back(byteCount);
    oss << byteToHex(byteCount);
    
    // 地址（高字节在前）
    uint8_t addrHigh = static_cast<uint8_t>((address >> 8) & 0xFF);
    uint8_t addrLow = static_cast<uint8_t>(address & 0xFF);
    bytes.push_back(addrHigh);
    bytes.push_back(addrLow);
    oss << byteToHex(addrHigh) << byteToHex(addrLow);
    
    // 记录类型
    bytes.push_back(type);
    oss << byteToHex(type);
    
    // 数据字节
    for (uint8_t byte : data) {
        bytes.push_back(byte);
        oss << byteToHex(byte);
    }
    
    // 计算并添加校验和
    uint8_t checksum = calculateChecksum(bytes);
    oss << byteToHex(checksum);
    
    return oss.str();
}

inline std::string HexWriter::generate(const std::vector<uint32_t>& words,
                                        uint32_t startAddress) {
    return generate(words, startAddress, 4096);  // 默认 4KB = 4096 words
}

/**
 * @brief 生成 Intel HEX 格式字符串（指定内存深度）
 * @param words 32位机器码序列
 * @param startAddress 起始地址（字地址，默认0）
 * @param depth 内存深度（总行数，默认4096）
 * @return Intel HEX 格式的完整字符串
 */
inline std::string HexWriter::generate(const std::vector<uint32_t>& words,
                                        uint32_t startAddress,
                                        uint32_t depth) {
    std::ostringstream oss;
    
    // 输出所有数据行（固定 depth 行）
    for (uint32_t i = 0; i < depth; i++) {
        uint32_t word = (i < words.size()) ? words[i] : 0x00000000;  // 不足的填充 0
        uint32_t byteAddress = (startAddress + i) * 4;
        
        // 如果地址超过 16 位，需要使用扩展地址记录
        if (byteAddress > 0xFFFF) {
            uint16_t upperAddr = static_cast<uint16_t>((byteAddress >> 16) & 0xFFFF);
            oss << generateExtendedAddressRecord(upperAddr) << "\n";
        }
        
        // 生成数据记录
        std::vector<uint8_t> data = wordToBytes(word);
        uint16_t addr16 = static_cast<uint16_t>(byteAddress & 0xFFFF);
        oss << generateDataRecord(addr16, data) << "\n";
    }
    
    // 添加 EOF 记录（第 4097 行）
    oss << generateEofRecord() << "\n";
    
    return oss.str();
}

inline bool HexWriter::writeFile(const std::vector<uint32_t>& words,
                                  const std::string& path,
                                  uint32_t startAddress) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << generate(words, startAddress);
    file.close();
    return true;
}

} // namespace assembler

#endif // HEX_WRITER_HPP
