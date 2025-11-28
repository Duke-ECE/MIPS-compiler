/**
 * @file asm_token.cpp
 * @brief 汇编器 Token 相关函数实现
 */

#include "assembler/asm_token.hpp"

namespace assembler {

std::string asmTokenTypeToString(AsmTokenType type) {
    switch (type) {
        case AsmTokenType::INSTRUCTION:  return "INSTRUCTION";
        case AsmTokenType::REGISTER:     return "REGISTER";
        case AsmTokenType::NUMBER:       return "NUMBER";
        case AsmTokenType::IDENTIFIER:   return "IDENTIFIER";
        case AsmTokenType::LABEL:        return "LABEL";
        case AsmTokenType::COMMA:        return "COMMA";
        case AsmTokenType::LPAREN:       return "LPAREN";
        case AsmTokenType::RPAREN:       return "RPAREN";
        case AsmTokenType::COLON:        return "COLON";
        case AsmTokenType::NEWLINE:      return "NEWLINE";
        case AsmTokenType::COMMENT:      return "COMMENT";
        case AsmTokenType::DIRECTIVE:    return "DIRECTIVE";
        case AsmTokenType::STRING:       return "STRING";
        case AsmTokenType::END_OF_FILE:  return "END_OF_FILE";
        case AsmTokenType::INVALID:      return "INVALID";
        default:                         return "UNKNOWN";
    }
}

} // namespace assembler
