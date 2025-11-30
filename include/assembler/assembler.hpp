/**
 * @file assembler.hpp
 * @brief 汇编器顶层接口
 * 
 * Assembler 类提供汇编器的统一入口，封装了完整的汇编流水线：
 * 
 * 流水线阶段：
 *   1. Lexer 阶段：汇编源代码 → Token 序列
 *   2. Parser 阶段：Token 序列 → 解析结果（指令列表 + 符号表 + 数据段）
 *   3. Encoder 阶段：解析结果 → 机器码
 * 
 * 使用示例：
 *   Assembler assembler;
 *   AssembleResult result = assembler.assemble(asmText);
 *   if (result.success) {
 *       // result.imem 包含指令内存
 *       // result.dmem 包含数据内存
 *   }
 */

#ifndef ASSEMBLER_ASSEMBLER_HPP
#define ASSEMBLER_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "assembler/asm_token.hpp"
#include "assembler/asm_parser.hpp"
#include "assembler/asm_encoder.hpp"

namespace assembler {

/**
 * @brief 汇编结果
 */
struct AssembleResult {
    bool success;                       // 是否成功
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
 * 提供简洁的接口将汇编源代码转换为机器码。
 * 
 * 设计原则：
 * - 只负责汇编逻辑，不涉及文件 I/O
 * - 文件读写由调用方（如 assembler_main.cpp）处理
 */
class Assembler {
public:
    /**
     * @brief 默认构造函数
     */
    Assembler() = default;
    
    /*****************************************
     * 流水线阶段接口
     *****************************************/
    
    /**
     * @brief 执行 Lexer 阶段
     * @param asmText 汇编源代码文本
     * @return Token 序列
     */
    std::vector<AsmToken> runLexer(const std::string& asmText);
    
    /**
     * @brief 执行 Parser 阶段
     * @param asmText 汇编源代码文本
     * @return 解析结果
     */
    ParseResult runParser(const std::string& asmText);
    
    /**
     * @brief 执行 Encoder 阶段
     * @param parseResult Parser 阶段的输出
     * @return 编码结果
     */
    EncodeResult runEncoder(const ParseResult& parseResult);
    
    /*****************************************
     * 顶层接口
     *****************************************/
    
    /**
     * @brief 执行完整汇编流水线
     * @param asmText 汇编源代码文本
     * @return 汇编结果，包含成功状态、机器码和错误信息
     */
    AssembleResult assemble(const std::string& asmText);
    
    /**
     * @brief 从汇编字符串生成机器码（简化接口）
     * @param asmText 汇编源代码文本
     * @return 指令内存机器码
     * @throws std::runtime_error 如果汇编失败
     */
    std::vector<uint32_t> assembleFromString(const std::string& asmText);
};

} // namespace assembler

#endif // ASSEMBLER_ASSEMBLER_HPP
