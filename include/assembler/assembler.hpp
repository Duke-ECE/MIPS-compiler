/**
 * @file assembler.hpp
 * @brief 汇编器顶层接口
 * 
 * Assembler 类提供汇编器的统一入口，封装了：
 * - 词法分析 (AsmLexer)
 * - 语法分析 (AsmParser)
 * - 编码生成 (AsmEncoder)
 * - 文件输出 (FileIO)
 * 
 * 使用示例：
 *   Assembler assembler;
 *   auto code = assembler.assemble(sourceText);
 *   assembler.writeHEX(code, "output.hex");
 *   assembler.writeMIF(code, "output.mif");
 */

#ifndef ASSEMBLER_ASSEMBLER_HPP
#define ASSEMBLER_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace assembler {

/**
 * @brief 汇编结果
 */
struct AssembleResult {
    bool success;                       // 是否成功
    std::vector<uint32_t> code;         // 机器码（兼容旧接口，包含imem）
    std::vector<uint32_t> imem;         // 指令内存（代码段）
    std::vector<uint32_t> dmem;         // 数据内存（数据段）
    std::vector<std::string> errors;    // 错误列表
    std::vector<std::string> warnings;  // 警告列表
    
    AssembleResult() : success(false) {}
    explicit operator bool() const { return success; }
};

/**
 * @brief 汇编器顶层类
 * 
 * 提供简洁的接口将汇编源代码转换为机器码，并支持多种输出格式。
 */
class Assembler {
public:
    /**
     * @brief 默认构造函数
     */
    Assembler() = default;
    
    /**
     * @brief 将汇编源代码转换为机器码
     * @param asmText 汇编源代码文本
     * @return 32位机器码序列
     * @throws std::runtime_error 如果汇编失败
     */
    std::vector<uint32_t> assemble(const std::string& asmText);
    
    /**
     * @brief 将汇编源代码转换为机器码（带详细结果）
     * @param asmText 汇编源代码文本
     * @return 汇编结果，包含成功状态、机器码和错误信息
     */
    AssembleResult assembleWithResult(const std::string& asmText);
    
    /**
     * @brief 将机器码写入 MIF 格式文件
     * @param code 32位机器码序列
     * @param path 输出文件路径
     */
    void writeMIF(const std::vector<uint32_t>& code, const std::string& path);
    
    /**
     * @brief 将机器码写入 Intel HEX 格式文件
     * @param code 32位机器码序列
     * @param path 输出文件路径
     */
    void writeHEX(const std::vector<uint32_t>& code, const std::string& path, bool wordAddressing = false);
    
    /**
     * @brief 获取最后一次汇编的错误信息
     * @return 错误信息列表
     */
    const std::vector<std::string>& getErrors() const { return lastErrors; }
    
    /**
     * @brief 获取最后一次汇编的警告信息
     * @return 警告信息列表
     */
    const std::vector<std::string>& getWarnings() const { return lastWarnings; }
    
    /**
     * @brief 检查最后一次汇编是否有错误
     * @return 如果有错误返回 true
     */
    bool hasErrors() const { return !lastErrors.empty(); }

private:
    std::vector<std::string> lastErrors;    // 最后一次汇编的错误
    std::vector<std::string> lastWarnings;  // 最后一次汇编的警告
};

} // namespace assembler

#endif // ASSEMBLER_ASSEMBLER_HPP
