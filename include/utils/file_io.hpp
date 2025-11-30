/**
 * @file file_io.hpp
 * @brief 文件输入输出工具类
 * 
 * 提供文件读写操作的封装，支持：
 * - 文本文件读取（用于读取汇编源文件 .s）
 * - Intel HEX 格式输出（.hex）
 * - MIF 格式输出（.mif，用于 Quartus FPGA）
 */

#ifndef UTILS_FILE_IO_HPP
#define UTILS_FILE_IO_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace utils {

/**
 * @brief 文件操作结果
 */
struct FileResult {
    bool success;           // 操作是否成功
    std::string error;      // 错误信息（如果失败）
    
    FileResult() : success(true) {}
    FileResult(bool s) : success(s) {}
    FileResult(const std::string& err) : success(false), error(err) {}
    
    explicit operator bool() const { return success; }
};

/**
 * @brief 文件输入输出工具类
 */
class FileIO {
public:
    // ==================== 文件读取 ====================
    
    /**
     * @brief 读取整个文本文件内容
     * @param path 文件路径
     * @return 文件内容字符串，失败返回 nullopt
     */
    static std::optional<std::string> readFile(const std::string& path);
    
    /**
     * @brief 读取文本文件（带异常抛出）
     * @param path 文件路径
     * @return 文件内容字符串
     * @throws std::runtime_error 如果读取失败
     */
    static std::string readText(const std::string& path);
    
    /**
     * @brief 读取文件的所有行
     * @param path 文件路径
     * @return 行列表，失败返回 nullopt
     */
    static std::optional<std::vector<std::string>> readLines(const std::string& path);
    
    // ==================== 文件写入 ====================
    
    /**
     * @brief 写入文本到文件
     * @param path 文件路径
     * @param content 文件内容
     * @return 操作结果
     */
    static FileResult writeFile(const std::string& path, const std::string& content);
    
    /**
     * @brief 写入文本到文件（带异常抛出）
     * @param path 文件路径
     * @param content 文件内容
     * @throws std::runtime_error 如果写入失败
     */
    static void writeText(const std::string& path, const std::string& content);
    
    /**
     * @brief 写入行列表到文件
     * @param path 文件路径
     * @param lines 行列表
     * @return 操作结果
     */
    static FileResult writeLines(const std::string& path, const std::vector<std::string>& lines);
    
    // ==================== Intel HEX 格式输出 ====================
    
    /**
     * @brief 将机器码写入 Intel HEX 格式文件
     * @param path 输出文件路径
     * @param code 32位机器码序列
     * @param startAddress 起始地址（字地址，默认0）
     * @return 操作结果
     */
    static FileResult writeHex(const std::string& path, 
                               const std::vector<uint32_t>& code,
                               uint32_t startAddress = 0,
                               bool wordAddressing = false);
    
    /**
     * @brief 生成 Intel HEX 格式字符串
     * @param code 32位机器码序列
     * @param startAddress 起始地址（字地址，默认0）
     * @return HEX 格式字符串
     */
    static std::string generateHex(const std::vector<uint32_t>& code,
                                   uint32_t startAddress = 0,
                                   bool wordAddressing = false);
    
    // ==================== MIF 格式输出 ====================
    
    /**
     * @brief 将机器码写入 MIF 格式文件（Quartus Memory Initialization File）
     * @param path 输出文件路径
     * @param code 32位机器码序列
     * @param depth 存储器深度（地址数量，默认自动计算）
     * @param width 数据宽度（位数，默认32）
     * @return 操作结果
     */
    static FileResult writeMif(const std::string& path,
                               const std::vector<uint32_t>& code,
                               uint32_t depth = 0,
                               uint32_t width = 32);
    
    /**
     * @brief 生成 MIF 格式字符串
     * @param code 32位机器码序列
     * @param depth 存储器深度（地址数量，默认自动计算）
     * @param width 数据宽度（位数，默认32）
     * @return MIF 格式字符串
     */
    static std::string generateMif(const std::vector<uint32_t>& code,
                                   uint32_t depth = 0,
                                   uint32_t width = 32);
    
    // ==================== 路径工具 ====================
    
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return 是否存在
     */
    static bool exists(const std::string& path);
    
    /**
     * @brief 获取文件扩展名
     * @param path 文件路径
     * @return 扩展名（包含点号，如 ".s"）
     */
    static std::string getExtension(const std::string& path);
    
    /**
     * @brief 获取文件名（不含路径）
     * @param path 文件路径
     * @return 文件名
     */
    static std::string getFileName(const std::string& path);
    
    /**
     * @brief 获取文件名（不含路径和扩展名）
     * @param path 文件路径
     * @return 文件基名
     */
    static std::string getBaseName(const std::string& path);
    
    /**
     * @brief 获取父目录路径
     * @param path 文件路径
     * @return 父目录路径
     */
    static std::string getDirectory(const std::string& path);
    
    /**
     * @brief 替换文件扩展名
     * @param path 原文件路径
     * @param newExt 新扩展名（包含点号，如 ".hex"）
     * @return 新文件路径
     */
    static std::string replaceExtension(const std::string& path, const std::string& newExt);
    
    /**
     * @brief 拼接路径
     * @param dir 目录路径
     * @param filename 文件名
     * @return 完整路径
     */
    static std::string joinPath(const std::string& dir, const std::string& filename);

private:
    // ==================== Intel HEX 辅助函数 ====================
    
    /**
     * @brief 计算 Intel HEX 校验和
     */
    static uint8_t calculateHexChecksum(const std::vector<uint8_t>& bytes);
    
    /**
     * @brief 生成单条 Intel HEX 记录
     */
    static std::string generateHexRecord(uint8_t type, uint16_t address,
                                          const std::vector<uint8_t>& data);
    
    /**
     * @brief 将 32 位字拆分为字节（大端序）
     */
    static std::vector<uint8_t> wordToBytes(uint32_t word);
};

} // namespace utils

#endif // UTILS_FILE_IO_HPP
