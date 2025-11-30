/**
 * @file asm_parser.cpp
 * @brief 汇编器语法分析器实现
 */

#include "assembler/asm_parser.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <sstream>

namespace assembler {

// ============================================================================
// 错误类型字符串转换
// ============================================================================

std::string asmErrorTypeToString(AsmErrorType type) {
    switch (type) {
        case AsmErrorType::SYNTAX_ERROR:           return "SYNTAX_ERROR";
        case AsmErrorType::UNDEFINED_LABEL:        return "UNDEFINED_LABEL";
        case AsmErrorType::DUPLICATE_LABEL:        return "DUPLICATE_LABEL";
        case AsmErrorType::INVALID_REGISTER:       return "INVALID_REGISTER";
        case AsmErrorType::INVALID_INSTRUCTION:    return "INVALID_INSTRUCTION";
        case AsmErrorType::INVALID_OPERAND:        return "INVALID_OPERAND";
        case AsmErrorType::IMMEDIATE_OUT_OF_RANGE: return "IMMEDIATE_OUT_OF_RANGE";
        case AsmErrorType::TARGET_OUT_OF_RANGE:    return "TARGET_OUT_OF_RANGE";
        case AsmErrorType::UNEXPECTED_TOKEN:       return "UNEXPECTED_TOKEN";
        case AsmErrorType::MISSING_OPERAND:        return "MISSING_OPERAND";
        case AsmErrorType::EXTRA_OPERAND:          return "EXTRA_OPERAND";
        default:                                   return "UNKNOWN_ERROR";
    }
}

// ============================================================================
// 构造函数
// ============================================================================

AsmParser::AsmParser(const std::string& source)
    : lexer(source), currentIndex(0), currentAddress(0), dataAddress(0),
      textStartAddress(0), dataStartAddress(0),
      inTextSection(true), inDataSection(false), pendingDataLabel("") {}

AsmParser::AsmParser(AsmLexer&& lex)
    : lexer(std::move(lex)), currentIndex(0), currentAddress(0), dataAddress(0),
      textStartAddress(0), dataStartAddress(0),
      inTextSection(true), inDataSection(false), pendingDataLabel("") {}

// ============================================================================
// 主解析函数
// ============================================================================

ParseResult AsmParser::parse() {
    ParseResult result;
    
    // 获取所有 token（包含换行和注释，用于生成 NOP）
    tokens = lexer.tokenize(true, true);
    currentIndex = 0;
    
    // 第一遍：收集标签
    firstPass();
    
    // 重置位置
    currentIndex = 0;
    currentAddress = textStartAddress;
    dataAddress = dataStartAddress;
    
    // 第二遍：解析指令
    secondPass();
    
    // 解析标签引用
    resolveLabels();
    
    // 填充结果
    result.success = errors.empty();
    result.instructions = instructions;
    result.dataWords = dataWords;
    result.symbols = symbolTable;
    result.errors = errors;
    result.warnings = warnings;
    result.textStart = textStartAddress;
    result.textSize = static_cast<uint32_t>(instructions.size());
    result.dataStart = dataStartAddress;
    result.dataSize = static_cast<uint32_t>(dataWords.size());
    
    return result;
}

// ============================================================================
// 第一遍扫描：收集标签
// ============================================================================

void AsmParser::firstPass() {
    currentAddress = textStartAddress;
    dataAddress = dataStartAddress;
    inTextSection = true;
    inDataSection = false;
    
    while (!isAtEnd()) {
        AsmToken token = currentToken();
        
        if (token.type == AsmTokenType::NEWLINE) {
            advance();
            continue;
        }
        
        if (token.type == AsmTokenType::LABEL) {
            // 检查标签是否已定义
            if (symbolTable.find(token.text) != symbolTable.end()) {
                reportError(AsmErrorType::DUPLICATE_LABEL, 
                    "标签 '" + token.text + "' 重复定义", token.line, token.column);
            } else {
                // 根据当前段确定地址和标签类型
                if (inDataSection) {
                    symbolTable[token.text] = SymbolInfo(token.text, dataAddress, token.line, true);
                } else {
                    symbolTable[token.text] = SymbolInfo(token.text, currentAddress, token.line, false);
                }
            }
            advance();
            continue;
        }
        
        if (token.type == AsmTokenType::COMMENT) {
            // 注释行也占用一个地址（生成 NOP）
            if (inTextSection && !inDataSection) {
                currentAddress++;
            }
            advance();
            continue;
        }
        
        if (token.type == AsmTokenType::DIRECTIVE) {
            // 处理汇编指令
            if (token.text == ".text") {
                inTextSection = true;
                inDataSection = false;
            } else if (token.text == ".data") {
                inTextSection = false;
                inDataSection = true;
            } else if (token.text == ".word" && inDataSection) {
                // .word 指令占用一个字
                dataAddress++;
                advance();
                // 跳过数值
                if (!isAtEnd() && currentToken().type == AsmTokenType::NUMBER) {
                    advance();
                }
                continue;
            }
            advance();
            continue;
        }
        
        if (token.type == AsmTokenType::INSTRUCTION) {
            // 指令占用一个字
            currentAddress++;
            // 跳过整行
            while (!isAtEnd() && currentToken().type != AsmTokenType::NEWLINE) {
                advance();
            }
            continue;
        }
        
        // 跳过其他 token
        advance();
    }
}

// ============================================================================
// 第二遍扫描：解析指令
// ============================================================================

void AsmParser::secondPass() {
    currentAddress = textStartAddress;
    dataAddress = dataStartAddress;
    inTextSection = true;
    inDataSection = false;
    pendingDataLabel = "";
    
    while (!isAtEnd()) {
        AsmToken token = currentToken();
        
        // 跳过换行
        if (token.type == AsmTokenType::NEWLINE) {
            advance();
            continue;
        }
        
        // 处理标签 - 生成 NOP 指令
        if (token.type == AsmTokenType::LABEL) {
            if (inDataSection) {
                // 数据段标签，保存以便关联到下一个 .word
                pendingDataLabel = token.text;
            } else {
                // 代码段标签，生成 NOP 指令（addi $zero, $zero, 0）
                ParsedInstruction nop;
                nop.line = token.line;
                nop.address = currentAddress;
                nop.opcode = isa::Opcode::ADDI;
                nop.type = isa::InstructionType::I_TYPE;
                nop.rd = isa::Register::R0;
                nop.rs = isa::Register::R0;
                nop.immediate = 0;
                nop.hasLabelRef = false;
                nop.originalText = token.text + ": (NOP)";
                instructions.push_back(nop);
                currentAddress++;
            }
            advance();
            continue;
        }
        
        // 处理注释 - 生成 NOP 指令
        if (token.type == AsmTokenType::COMMENT) {
            if (inTextSection && !inDataSection) {
                // 代码段注释，生成 NOP 指令（addi $zero, $zero, 0）
                ParsedInstruction nop;
                nop.line = token.line;
                nop.address = currentAddress;
                nop.opcode = isa::Opcode::ADDI;
                nop.type = isa::InstructionType::I_TYPE;
                nop.rd = isa::Register::R0;
                nop.rs = isa::Register::R0;
                nop.immediate = 0;
                nop.hasLabelRef = false;
                nop.originalText = token.text + " (NOP)";
                instructions.push_back(nop);
                currentAddress++;
            }
            advance();
            continue;
        }
        
        // 处理汇编指令
        if (token.type == AsmTokenType::DIRECTIVE) {
            if (token.text == ".text") {
                inTextSection = true;
                inDataSection = false;
            } else if (token.text == ".data") {
                inTextSection = false;
                inDataSection = true;
            } else if (token.text == ".word" && inDataSection) {
                // 解析 .word 指令
                int wordLine = token.line;
                advance(); // 消耗 .word
                
                // 期望一个数值
                if (!isAtEnd() && currentToken().type == AsmTokenType::NUMBER) {
                    uint32_t value = static_cast<uint32_t>(currentToken().numericValue);
                    DataWord dw(dataAddress, value, wordLine, pendingDataLabel);
                    dataWords.push_back(dw);
                    dataAddress++;
                    pendingDataLabel = ""; // 清空待关联的标签
                    advance(); // 消耗数值
                } else {
                    reportError(AsmErrorType::MISSING_OPERAND,
                        ".word 指令需要一个数值", wordLine, token.column);
                }
                continue;
            }
            advance();
            continue;
        }
        
        // 解析指令（仅在代码段）
        if (token.type == AsmTokenType::INSTRUCTION) {
            if (inDataSection) {
                reportError(AsmErrorType::SYNTAX_ERROR,
                    "数据段中不能有指令: '" + token.text + "'", token.line, token.column);
                synchronize();
                continue;
            }
            auto instr = parseInstruction();
            if (instr.has_value()) {
                instr->address = currentAddress;
                instructions.push_back(*instr);
                currentAddress++;
            } else {
                // 错误恢复：跳到下一行
                synchronize();
            }
            continue;
        }
        
        // 在数据段跳过数值（可能是没有 .word 前缀的数据，兼容某些格式）
        if (token.type == AsmTokenType::NUMBER && inDataSection) {
            advance();
            continue;
        }
        
        // 意外的 token
        reportError(AsmErrorType::UNEXPECTED_TOKEN, 
            "意外的 token: '" + token.text + "'", token.line, token.column);
        advance();
    }
}

// ============================================================================
// 解析标签引用
// ============================================================================

void AsmParser::resolveLabels() {
    for (auto& instr : instructions) {
        if (!instr.hasLabelRef) continue;
        
        auto addr = lookupLabel(instr.labelRef);
        if (!addr.has_value()) {
            reportError(AsmErrorType::UNDEFINED_LABEL,
                "未定义的标签: '" + instr.labelRef + "'", instr.line, 0);
            continue;
        }
        
        // 根据指令类型设置目标
        if (instr.type == isa::InstructionType::J_TYPE) {
            // J-type: 绝对地址
            if (!isTargetInRange(*addr)) {
                reportError(AsmErrorType::TARGET_OUT_OF_RANGE,
                    "跳转目标超出范围: " + std::to_string(*addr), instr.line, 0);
            }
            instr.target = *addr;
        } else if (instr.type == isa::InstructionType::I_TYPE) {
            // I-type 分支: 相对偏移
            int32_t offset = calculateBranchOffset(instr.address, *addr);
            if (!isImmediateInRange(offset)) {
                reportError(AsmErrorType::IMMEDIATE_OUT_OF_RANGE,
                    "分支偏移超出范围: " + std::to_string(offset), instr.line, 0);
            }
            instr.immediate = offset;
        }
        
        instr.hasLabelRef = false;
    }
}

// ============================================================================
// 指令解析
// ============================================================================

std::optional<ParsedInstruction> AsmParser::parseInstruction() {
    AsmToken instrToken = currentToken();
    
    if (instrToken.type != AsmTokenType::INSTRUCTION) {
        reportError(AsmErrorType::INVALID_INSTRUCTION,
            "期望指令，得到: '" + instrToken.text + "'", instrToken.line, instrToken.column);
        return std::nullopt;
    }
    
    isa::Opcode opcode = isa::stringToOpcode(instrToken.text);
    if (opcode == isa::Opcode::INVALID) {
        reportError(AsmErrorType::INVALID_INSTRUCTION,
            "无效的指令: '" + instrToken.text + "'", instrToken.line, instrToken.column);
        return std::nullopt;
    }
    
    advance(); // 消耗指令 token
    
    isa::InstructionType type = isa::getInstructionType(opcode);
    
    std::optional<ParsedInstruction> result;
    switch (type) {
        case isa::InstructionType::R_TYPE:
            result = parseRTypeInstruction(opcode);
            break;
        case isa::InstructionType::I_TYPE:
            result = parseITypeInstruction(opcode);
            break;
        case isa::InstructionType::J_TYPE:
            result = parseJTypeInstruction(opcode);
            break;
        default:
            reportError(AsmErrorType::INVALID_INSTRUCTION,
                "未知的指令类型", instrToken.line, instrToken.column);
            return std::nullopt;
    }
    
    if (result.has_value()) {
        result->line = instrToken.line;
        result->originalText = instrToken.text;
    }
    
    return result;
}

// ============================================================================
// R-type 指令解析: add $rd, $rs, $rt
// ============================================================================

std::optional<ParsedInstruction> AsmParser::parseRTypeInstruction(isa::Opcode opcode) {
    ParsedInstruction instr;
    instr.opcode = opcode;
    instr.type = isa::InstructionType::R_TYPE;
    
    // 解析 $rd
    instr.rd = parseRegister();
    if (instr.rd == isa::Register::INVALID) {
        return std::nullopt;
    }
    
    // 期望逗号
    if (!match(AsmTokenType::COMMA)) {
        reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
        return std::nullopt;
    }
    
    // 解析 $rs
    instr.rs = parseRegister();
    if (instr.rs == isa::Register::INVALID) {
        return std::nullopt;
    }
    
    // 期望逗号
    if (!match(AsmTokenType::COMMA)) {
        reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
        return std::nullopt;
    }
    
    // 解析 $rt
    instr.rt = parseRegister();
    if (instr.rt == isa::Register::INVALID) {
        return std::nullopt;
    }
    
    return instr;
}

// ============================================================================
// I-type 指令解析
// ============================================================================

std::optional<ParsedInstruction> AsmParser::parseITypeInstruction(isa::Opcode opcode) {
    ParsedInstruction instr;
    instr.opcode = opcode;
    instr.type = isa::InstructionType::I_TYPE;
    
    // 根据具体指令解析不同的格式
    switch (opcode) {
        case isa::Opcode::LW:
        case isa::Opcode::SW: {
            // lw/sw $rd, offset($rs)
            instr.rd = parseRegister();
            if (instr.rd == isa::Register::INVALID) return std::nullopt;
            
            if (!match(AsmTokenType::COMMA)) {
                reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
                return std::nullopt;
            }
            
            if (!parseMemoryOperand(instr.immediate, instr.rs)) {
                return std::nullopt;
            }
            break;
        }
        
        case isa::Opcode::ADDI: {
            // addi $rd, $rs, imm
            instr.rd = parseRegister();
            if (instr.rd == isa::Register::INVALID) return std::nullopt;
            
            if (!match(AsmTokenType::COMMA)) {
                reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
                return std::nullopt;
            }
            
            instr.rs = parseRegister();
            if (instr.rs == isa::Register::INVALID) return std::nullopt;
            
            if (!match(AsmTokenType::COMMA)) {
                reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
                return std::nullopt;
            }
            
            // 解析立即数
            AsmToken token = currentToken();
            if (token.type == AsmTokenType::NUMBER) {
                instr.immediate = static_cast<int32_t>(token.numericValue);
                if (!isImmediateInRange(instr.immediate)) {
                    reportError(AsmErrorType::IMMEDIATE_OUT_OF_RANGE,
                        "立即数超出范围: " + std::to_string(instr.immediate));
                }
                advance();
            } else {
                reportError(AsmErrorType::INVALID_OPERAND, "期望立即数");
                return std::nullopt;
            }
            break;
        }
        
        case isa::Opcode::BEQ:
        case isa::Opcode::BGT: {
            // beq/bgt $rd, $rs, label/offset
            instr.rd = parseRegister();
            if (instr.rd == isa::Register::INVALID) return std::nullopt;
            
            if (!match(AsmTokenType::COMMA)) {
                reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
                return std::nullopt;
            }
            
            instr.rs = parseRegister();
            if (instr.rs == isa::Register::INVALID) return std::nullopt;
            
            if (!match(AsmTokenType::COMMA)) {
                reportError(AsmErrorType::SYNTAX_ERROR, "期望 ','");
                return std::nullopt;
            }
            
            // 解析标签或偏移量
            if (!parseImmediateOrLabel(instr.immediate, instr.labelRef)) {
                return std::nullopt;
            }
            instr.hasLabelRef = !instr.labelRef.empty();
            break;
        }
        
        case isa::Opcode::JR: {
            // jr $rd
            instr.rd = parseRegister();
            if (instr.rd == isa::Register::INVALID) return std::nullopt;
            instr.rs = isa::Register::R0;
            instr.immediate = 0;
            break;
        }
        
        case isa::Opcode::INPUT:
        case isa::Opcode::OUTPUT: {
            // input/output $rd
            instr.rd = parseRegister();
            if (instr.rd == isa::Register::INVALID) return std::nullopt;
            instr.rs = isa::Register::R0;
            instr.immediate = 0;
            break;
        }
        
        default:
            reportError(AsmErrorType::INVALID_INSTRUCTION, "未处理的 I-type 指令");
            return std::nullopt;
    }
    
    return instr;
}

// ============================================================================
// J-type 指令解析: j label / jal label
// ============================================================================

std::optional<ParsedInstruction> AsmParser::parseJTypeInstruction(isa::Opcode opcode) {
    ParsedInstruction instr;
    instr.opcode = opcode;
    instr.type = isa::InstructionType::J_TYPE;
    
    // 解析目标（标签或地址）
    AsmToken token = currentToken();
    
    if (token.type == AsmTokenType::IDENTIFIER) {
        // 标签引用
        instr.labelRef = token.text;
        instr.hasLabelRef = true;
        advance();
    } else if (token.type == AsmTokenType::NUMBER) {
        // 直接地址
        instr.target = static_cast<uint32_t>(token.numericValue);
        if (!isTargetInRange(instr.target)) {
            reportError(AsmErrorType::TARGET_OUT_OF_RANGE,
                "跳转目标超出范围: " + std::to_string(instr.target));
        }
        instr.hasLabelRef = false;
        advance();
    } else {
        reportError(AsmErrorType::INVALID_OPERAND, "期望标签或地址");
        return std::nullopt;
    }
    
    return instr;
}

// ============================================================================
// 内存操作数解析: offset($rs)
// ============================================================================

bool AsmParser::parseMemoryOperand(int32_t& offset, isa::Register& rs) {
    // 解析 offset（可以是数字或省略为0）
    AsmToken token = currentToken();
    
    if (token.type == AsmTokenType::NUMBER) {
        offset = static_cast<int32_t>(token.numericValue);
        if (!isImmediateInRange(offset)) {
            reportError(AsmErrorType::IMMEDIATE_OUT_OF_RANGE,
                "偏移量超出范围: " + std::to_string(offset));
            return false;
        }
        advance();
    } else if (token.type == AsmTokenType::LPAREN) {
        // 省略 offset，默认为 0
        offset = 0;
    } else {
        reportError(AsmErrorType::INVALID_OPERAND, "期望偏移量或 '('");
        return false;
    }
    
    // 期望 '('
    if (!match(AsmTokenType::LPAREN)) {
        reportError(AsmErrorType::SYNTAX_ERROR, "期望 '('");
        return false;
    }
    
    // 解析 $rs
    rs = parseRegister();
    if (rs == isa::Register::INVALID) {
        return false;
    }
    
    // 期望 ')'
    if (!match(AsmTokenType::RPAREN)) {
        reportError(AsmErrorType::SYNTAX_ERROR, "期望 ')'");
        return false;
    }
    
    return true;
}

// ============================================================================
// Token 操作
// ============================================================================

AsmToken AsmParser::currentToken() const {
    if (currentIndex >= tokens.size()) {
        return AsmToken(AsmTokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentIndex];
}

AsmToken AsmParser::peekToken() const {
    if (currentIndex + 1 >= tokens.size()) {
        return AsmToken(AsmTokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens[currentIndex + 1];
}

void AsmParser::advance() {
    if (currentIndex < tokens.size()) {
        currentIndex++;
    }
}

AsmToken AsmParser::consume(AsmTokenType type, const std::string& errorMsg) {
    if (check(type)) {
        AsmToken token = currentToken();
        advance();
        return token;
    }
    reportError(AsmErrorType::UNEXPECTED_TOKEN, errorMsg);
    return AsmToken(AsmTokenType::INVALID, "", currentToken().line, currentToken().column);
}

bool AsmParser::check(AsmTokenType type) const {
    if (isAtEnd()) return false;
    return currentToken().type == type;
}

bool AsmParser::match(AsmTokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void AsmParser::skipNewlines() {
    while (check(AsmTokenType::NEWLINE)) {
        advance();
    }
}

void AsmParser::synchronize() {
    // 跳到下一行
    while (!isAtEnd() && currentToken().type != AsmTokenType::NEWLINE) {
        advance();
    }
    if (!isAtEnd()) {
        advance(); // 跳过换行符
    }
}

bool AsmParser::isAtEnd() const {
    return currentIndex >= tokens.size() || currentToken().type == AsmTokenType::END_OF_FILE;
}

// ============================================================================
// 辅助函数
// ============================================================================

isa::Register AsmParser::parseRegister() {
    AsmToken token = currentToken();
    
    if (token.type != AsmTokenType::REGISTER) {
        reportError(AsmErrorType::INVALID_REGISTER,
            "期望寄存器，得到: '" + token.text + "'", token.line, token.column);
        return isa::Register::INVALID;
    }
    
    isa::Register reg = isa::stringToRegister(token.text);
    if (reg == isa::Register::INVALID) {
        reportError(AsmErrorType::INVALID_REGISTER,
            "无效的寄存器: '" + token.text + "'", token.line, token.column);
        return isa::Register::INVALID;
    }
    
    advance();
    return reg;
}

bool AsmParser::parseImmediateOrLabel(int32_t& value, std::string& labelRef) {
    AsmToken token = currentToken();
    
    if (token.type == AsmTokenType::NUMBER) {
        value = static_cast<int32_t>(token.numericValue);
        labelRef = "";
        advance();
        return true;
    } else if (token.type == AsmTokenType::IDENTIFIER) {
        labelRef = token.text;
        value = 0; // 将在 resolveLabels 中设置
        advance();
        return true;
    }
    
    reportError(AsmErrorType::INVALID_OPERAND, "期望立即数或标签");
    return false;
}

std::optional<uint32_t> AsmParser::lookupLabel(const std::string& label) const {
    auto it = symbolTable.find(label);
    if (it != symbolTable.end()) {
        return it->second.address;
    }
    return std::nullopt;
}

int32_t AsmParser::calculateBranchOffset(uint32_t currentAddr, uint32_t targetAddr) const {
    // 分支偏移 = 目标地址 - (当前地址 + 1)
    // 因为 PC 在执行分支时已经指向下一条指令
    return static_cast<int32_t>(targetAddr) - static_cast<int32_t>(currentAddr + 1);
}

bool AsmParser::isImmediateInRange(int32_t value) const {
    // 17-bit signed: -65536 to +65535
    return value >= -(1 << 16) && value <= ((1 << 16) - 1);
}

bool AsmParser::isTargetInRange(uint32_t target) const {
    // 27-bit unsigned
    return target <= ((1u << 27) - 1);
}

// ============================================================================
// 错误处理
// ============================================================================

void AsmParser::reportError(AsmErrorType type, const std::string& message) {
    AsmToken token = currentToken();
    reportError(type, message, token.line, token.column);
}

void AsmParser::reportError(AsmErrorType type, const std::string& message, int line, int column) {
    errors.emplace_back(type, message, line, column);
}

void AsmParser::reportWarning(const std::string& message) {
    AsmToken token = currentToken();
    std::stringstream ss;
    ss << "警告 [行 " << token.line << "]: " << message;
    warnings.push_back(ss.str());
}

} // namespace assembler
