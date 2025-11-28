/**
 * @file asm_lexer.cpp
 * @brief 汇编器词法分析器实现
 */

#include "assembler/asm_lexer.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <cctype>

namespace assembler {

// ============================================================================
// 构造函数和重置
// ============================================================================

AsmLexer::AsmLexer(const std::string& source)
    : source(source), pos(0), line(1), column(1), 
      hasPeekedToken(false), peekedToken() {}

void AsmLexer::reset() {
    pos = 0;
    line = 1;
    column = 1;
    hasPeekedToken = false;
    peekedToken = AsmToken();
}

// ============================================================================
// 字符操作
// ============================================================================

bool AsmLexer::isAtEnd() const {
    return pos >= source.size();
}

bool AsmLexer::eof() const {
    return isAtEnd() && !hasPeekedToken;
}

char AsmLexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char AsmLexer::peekNext() const {
    if (pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
}

char AsmLexer::get() {
    char c = peek();
    advance();
    return c;
}

void AsmLexer::advance() {
    if (!isAtEnd()) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
}

// ============================================================================
// 字符判断辅助函数
// ============================================================================

bool AsmLexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool AsmLexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool AsmLexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool AsmLexer::isHexDigit(char c) const {
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool AsmLexer::isBinaryDigit(char c) const {
    return c == '0' || c == '1';
}

bool AsmLexer::isInstruction(const std::string& str) const {
    // 使用 ISA 模块的 stringToOpcode 函数判断
    return isa::stringToOpcode(str) != isa::Opcode::INVALID;
}

// ============================================================================
// 跳过处理
// ============================================================================

void AsmLexer::skipWhitespaceExceptNewline() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else {
            break;
        }
    }
}

void AsmLexer::skipLineComment() {
    // 跳过直到行尾
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

// ============================================================================
// 数字解析
// ============================================================================

int64_t AsmLexer::parseDecimal(const std::string& str) const {
    int64_t result = 0;
    bool negative = false;
    size_t i = 0;
    
    if (!str.empty() && str[0] == '-') {
        negative = true;
        i = 1;
    } else if (!str.empty() && str[0] == '+') {
        i = 1;
    }
    
    for (; i < str.size(); i++) {
        result = result * 10 + (str[i] - '0');
    }
    
    return negative ? -result : result;
}

int64_t AsmLexer::parseHexadecimal(const std::string& str) const {
    int64_t result = 0;
    // 跳过 "0x" 或 "0X" 前缀
    size_t i = 2;
    
    for (; i < str.size(); i++) {
        char c = str[i];
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            break;
        }
        result = result * 16 + digit;
    }
    
    return result;
}

int64_t AsmLexer::parseBinary(const std::string& str) const {
    int64_t result = 0;
    // 跳过 "0b" 或 "0B" 前缀
    size_t i = 2;
    
    for (; i < str.size(); i++) {
        if (str[i] == '0' || str[i] == '1') {
            result = result * 2 + (str[i] - '0');
        } else {
            break;
        }
    }
    
    return result;
}

// ============================================================================
// Token 解析
// ============================================================================

AsmToken AsmLexer::parseRegister() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    
    // 消耗 '$'
    text += get();
    
    // 读取寄存器名
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        text += get();
    }
    
    // 验证是否为有效寄存器
    if (isa::stringToRegister(text) != isa::Register::INVALID) {
        return AsmToken(AsmTokenType::REGISTER, text, startLine, startColumn);
    }
    
    // 无效寄存器
    return AsmToken(AsmTokenType::INVALID, text, startLine, startColumn);
}

AsmToken AsmLexer::parseNumber() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    bool negative = false;
    
    // 处理正负号
    if (peek() == '-') {
        negative = true;
        text += get();
    } else if (peek() == '+') {
        text += get();
    }
    
    // 检查是否为十六进制或二进制
    if (peek() == '0' && (peekNext() == 'x' || peekNext() == 'X')) {
        // 十六进制
        text += get(); // '0'
        text += get(); // 'x'
        while (!isAtEnd() && isHexDigit(peek())) {
            text += get();
        }
        int64_t value = parseHexadecimal(text);
        if (negative) value = -value;
        return AsmToken(AsmTokenType::NUMBER, text, startLine, startColumn, value);
    } else if (peek() == '0' && (peekNext() == 'b' || peekNext() == 'B')) {
        // 二进制
        text += get(); // '0'
        text += get(); // 'b'
        while (!isAtEnd() && isBinaryDigit(peek())) {
            text += get();
        }
        int64_t value = parseBinary(text);
        if (negative) value = -value;
        return AsmToken(AsmTokenType::NUMBER, text, startLine, startColumn, value);
    } else {
        // 十进制
        while (!isAtEnd() && isDigit(peek())) {
            text += get();
        }
        int64_t value = parseDecimal(text);
        return AsmToken(AsmTokenType::NUMBER, text, startLine, startColumn, value);
    }
}

AsmToken AsmLexer::parseIdentifierOrLabel() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    
    // 读取标识符
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        text += get();
    }
    
    // 跳过空白（不包括换行）
    skipWhitespaceExceptNewline();
    
    // 检查是否为标签定义（后面跟着冒号）
    if (!isAtEnd() && peek() == ':') {
        advance(); // 消耗冒号
        return AsmToken(AsmTokenType::LABEL, text, startLine, startColumn);
    }
    
    // 检查是否为指令
    if (isInstruction(text)) {
        return AsmToken(AsmTokenType::INSTRUCTION, text, startLine, startColumn);
    }
    
    // 否则为标识符（标签引用）
    return AsmToken(AsmTokenType::IDENTIFIER, text, startLine, startColumn);
}

AsmToken AsmLexer::parseDirective() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    
    // 消耗 '.'
    text += get();
    
    // 读取指令名
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        text += get();
    }
    
    return AsmToken(AsmTokenType::DIRECTIVE, text, startLine, startColumn);
}

AsmToken AsmLexer::parseString() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    
    char quote = get(); // 消耗开头的引号
    text += quote;
    
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\\' && peekNext() != '\0') {
            // 处理转义字符
            text += get(); // '\'
            text += get(); // 被转义的字符
        } else if (peek() == '\n') {
            // 字符串不能跨行
            return AsmToken(AsmTokenType::INVALID, text, startLine, startColumn);
        } else {
            text += get();
        }
    }
    
    if (isAtEnd()) {
        // 未闭合的字符串
        return AsmToken(AsmTokenType::INVALID, text, startLine, startColumn);
    }
    
    text += get(); // 消耗结尾的引号
    return AsmToken(AsmTokenType::STRING, text, startLine, startColumn);
}

AsmToken AsmLexer::parseComment() {
    int startLine = line;
    int startColumn = column;
    std::string text;
    
    // 读取整行注释
    while (!isAtEnd() && peek() != '\n') {
        text += get();
    }
    
    return AsmToken(AsmTokenType::COMMENT, text, startLine, startColumn);
}

// ============================================================================
// 主解析函数
// ============================================================================

AsmToken AsmLexer::nextToken() {
    // 如果有预读的 token，直接返回
    if (hasPeekedToken) {
        hasPeekedToken = false;
        return peekedToken;
    }
    
    // 跳过空白（保留换行）
    skipWhitespaceExceptNewline();
    
    if (isAtEnd()) {
        return AsmToken(AsmTokenType::END_OF_FILE, "", line, column);
    }
    
    int startLine = line;
    int startColumn = column;
    char c = peek();
    
    // 换行符
    if (c == '\n') {
        advance();
        return AsmToken(AsmTokenType::NEWLINE, "\\n", startLine, startColumn);
    }
    
    // 注释
    if (c == '#' || c == ';') {
        return parseComment();
    }
    
    // 寄存器
    if (c == '$') {
        return parseRegister();
    }
    
    // 汇编指令
    if (c == '.') {
        return parseDirective();
    }
    
    // 字符串
    if (c == '"' || c == '\'') {
        return parseString();
    }
    
    // 数字（包括负数）
    if (isDigit(c) || ((c == '-' || c == '+') && isDigit(peekNext()))) {
        return parseNumber();
    }
    
    // 标识符、标签、指令
    if (isAlpha(c)) {
        return parseIdentifierOrLabel();
    }
    
    // 单字符 token
    switch (c) {
        case ',':
            advance();
            return AsmToken(AsmTokenType::COMMA, ",", startLine, startColumn);
        case '(':
            advance();
            return AsmToken(AsmTokenType::LPAREN, "(", startLine, startColumn);
        case ')':
            advance();
            return AsmToken(AsmTokenType::RPAREN, ")", startLine, startColumn);
        case ':':
            advance();
            return AsmToken(AsmTokenType::COLON, ":", startLine, startColumn);
    }
    
    // 无法识别的字符
    std::string invalidChar(1, get());
    return AsmToken(AsmTokenType::INVALID, invalidChar, startLine, startColumn);
}

AsmToken AsmLexer::peekToken() {
    if (!hasPeekedToken) {
        peekedToken = nextToken();
        hasPeekedToken = true;
    }
    return peekedToken;
}

std::vector<AsmToken> AsmLexer::tokenize(bool includeNewlines, bool includeComments) {
    std::vector<AsmToken> tokens;
    
    while (true) {
        AsmToken token = nextToken();
        
        // 根据参数决定是否包含换行和注释
        if (token.type == AsmTokenType::NEWLINE && !includeNewlines) {
            continue;
        }
        if (token.type == AsmTokenType::COMMENT && !includeComments) {
            continue;
        }
        
        tokens.push_back(token);
        
        if (token.type == AsmTokenType::END_OF_FILE) {
            break;
        }
    }
    
    return tokens;
}

} // namespace assembler
