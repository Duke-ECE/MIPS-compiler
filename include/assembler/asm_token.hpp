/**
 * @file asm_token.hpp
 * @brief 汇编器词法分析器的 Token 定义
 * 
 * 定义汇编语言的所有词法单元类型
 */

#ifndef ASSEMBLER_ASM_TOKEN_HPP
#define ASSEMBLER_ASM_TOKEN_HPP

#include <string>

namespace assembler {

/**
 * @brief 汇编语言的 Token 类型枚举
 */
enum class AsmTokenType {
    // ============== 指令助记符 ==============
    INSTRUCTION,    // add, sub, lw, sw, beq, j, jal 等指令

    // ============== 操作数 ==============
    REGISTER,       // 寄存器：$r0, $r1, ..., $r31, $zero, $ra, $sp 等
    NUMBER,         // 立即数：123, -456, 0x1A, 0b1010
    IDENTIFIER,     // 标签引用：loop, end, func_name

    // ============== 标签定义 ==============
    LABEL,          // 标签定义：main:, loop:, end:

    // ============== 分隔符 ==============
    COMMA,          // ,
    LPAREN,         // (
    RPAREN,         // )
    COLON,          // :
    NEWLINE,        // 换行符（汇编语言中换行是有意义的）

    // ============== 特殊 Token ==============
    COMMENT,        // 注释：# this is a comment 或 ; comment
    DIRECTIVE,      // 汇编指令：.data, .text, .word 等
    STRING,         // 字符串字面量："hello"

    // ============== 结束和错误 ==============
    END_OF_FILE,    // 文件结束
    INVALID         // 无效 token
};

/**
 * @brief 汇编语言的 Token 结构
 */
struct AsmToken {
    AsmTokenType type;      // Token 类型
    std::string text;       // Token 原始文本
    int line;               // 行号（从1开始）
    int column;             // 列号（从1开始）
    
    // 对于 NUMBER 类型，存储解析后的数值
    int64_t numericValue;
    
    /**
     * @brief 构造函数
     */
    AsmToken(AsmTokenType t, const std::string& txt, int ln, int col, int64_t val = 0)
        : type(t), text(txt), line(ln), column(col), numericValue(val) {}
    
    /**
     * @brief 默认构造函数
     */
    AsmToken() : type(AsmTokenType::INVALID), text(""), line(0), column(0), numericValue(0) {}
};

/**
 * @brief 将 Token 类型转换为字符串（用于调试）
 * @param type Token 类型
 * @return 类型的字符串表示
 */
std::string asmTokenTypeToString(AsmTokenType type);

} // namespace assembler

#endif // ASSEMBLER_ASM_TOKEN_HPP
