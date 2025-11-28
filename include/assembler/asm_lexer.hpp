/**
 * @file asm_lexer.hpp
 * @brief 汇编器词法分析器
 * 
 * AsmLexer 负责将汇编源代码字符串转换为 Token 序列。
 * 支持的语法：
 * - 指令：add, sub, and, or, sll, srl, addi, lw, sw, beq, bgt, jr, j, jal, input, output
 * - 寄存器：$r0-$r31, $zero, $ra, $sp 等别名
 * - 立即数：十进制、十六进制(0x)、二进制(0b)
 * - 标签：identifier: 
 * - 注释：# 或 ; 开头到行尾
 * - 汇编指令：.data, .text, .word 等
 * 
 * 示例输入：
 *   main:
 *       add $r1, $r2, $r3    # R-type 指令
 *       lw $r4, 100($r5)     # I-type 指令
 *       j end                # J-type 指令
 *   end:
 */

#ifndef ASSEMBLER_ASM_LEXER_HPP
#define ASSEMBLER_ASM_LEXER_HPP

#include <string>
#include <vector>
#include "asm_token.hpp"

namespace assembler {

/**
 * @brief 汇编器词法分析器类
 */
class AsmLexer {
public:
    /**
     * @brief 构造函数
     * @param source 汇编源代码字符串
     */
    explicit AsmLexer(const std::string& source);

    /**
     * @brief 获取下一个 Token
     * @return 下一个 Token
     */
    AsmToken nextToken();

    /**
     * @brief 查看下一个 Token（不消耗）
     * @return 下一个 Token
     */
    AsmToken peekToken();

    /**
     * @brief 判断是否到达输入末尾
     * @return 如果到达末尾返回 true
     */
    bool eof() const;

    /**
     * @brief 一次性获取所有 Token
     * @param includeNewlines 是否包含换行 Token，默认 true
     * @param includeComments 是否包含注释 Token，默认 false
     * @return Token 向量
     */
    std::vector<AsmToken> tokenize(bool includeNewlines = true, bool includeComments = false);

    /**
     * @brief 获取当前行号
     * @return 当前行号
     */
    int currentLine() const { return line; }

    /**
     * @brief 获取当前列号
     * @return 当前列号
     */
    int currentColumn() const { return column; }

    /**
     * @brief 重置词法分析器状态
     */
    void reset();

private:
    // 字符操作
    char peek() const;                  // 查看当前字符
    char peekNext() const;              // 查看下一个字符
    char get();                         // 获取当前字符并前进
    void advance();                     // 前进一个字符
    
    // 跳过处理
    void skipWhitespaceExceptNewline(); // 跳过空白（保留换行）
    void skipLineComment();             // 跳过行注释
    
    // Token 解析
    AsmToken parseRegister();           // 解析寄存器
    AsmToken parseNumber();             // 解析数字
    AsmToken parseIdentifierOrLabel();  // 解析标识符或标签
    AsmToken parseDirective();          // 解析汇编指令
    AsmToken parseString();             // 解析字符串
    AsmToken parseComment();            // 解析注释
    
    // 辅助函数
    bool isAtEnd() const;
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isHexDigit(char c) const;
    bool isBinaryDigit(char c) const;
    bool isInstruction(const std::string& str) const;
    
    // 数字解析
    int64_t parseDecimal(const std::string& str) const;
    int64_t parseHexadecimal(const std::string& str) const;
    int64_t parseBinary(const std::string& str) const;

private:
    std::string source;     // 源代码
    size_t pos;             // 当前位置
    int line;               // 当前行号
    int column;             // 当前列号
    
    bool hasPeekedToken;    // 是否有预读的 Token
    AsmToken peekedToken;   // 预读的 Token
};

} // namespace assembler

#endif // ASSEMBLER_ASM_LEXER_HPP
