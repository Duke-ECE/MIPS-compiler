/**
 * @file asm_encoder.cpp
 * @brief 汇编器编码器实现
 */

#include "assembler/asm_encoder.hpp"
#include "isa/Instructions.hpp"
#include <sstream>
#include <iomanip>
#include <bitset>

namespace assembler {

// ============================================================================
// 错误类型字符串转换
// ============================================================================

std::string encoderErrorTypeToString(EncoderErrorType type) {
    switch (type) {
        case EncoderErrorType::INVALID_OPCODE:
            return "INVALID_OPCODE";
        case EncoderErrorType::INVALID_REGISTER:
            return "INVALID_REGISTER";
        case EncoderErrorType::IMMEDIATE_OUT_OF_RANGE:
            return "IMMEDIATE_OUT_OF_RANGE";
        case EncoderErrorType::TARGET_OUT_OF_RANGE:
            return "TARGET_OUT_OF_RANGE";
        case EncoderErrorType::UNRESOLVED_LABEL:
            return "UNRESOLVED_LABEL";
        case EncoderErrorType::ENCODING_FAILED:
            return "ENCODING_FAILED";
        default:
            return "UNKNOWN";
    }
}

// ============================================================================
// 构造函数
// ============================================================================

AsmEncoder::AsmEncoder() {}

// ============================================================================
// 主编码函数
// ============================================================================

EncodeResult AsmEncoder::encode(const ParseResult& parseResult) {
    EncodeResult result;
    errors.clear();
    warnings.clear();
    
    // 如果解析失败，直接返回
    if (!parseResult.success) {
        result.success = false;
        reportError(EncoderErrorType::ENCODING_FAILED, "解析阶段失败，无法编码");
        result.errors = errors;
        return result;
    }
    
    result.textStartAddress = parseResult.textStart;
    result.totalInstructions = 0;
    
    // 遍历所有指令进行编码
    for (const auto& instr : parseResult.instructions) {
        auto machineCode = encodeInstruction(instr, parseResult.symbols);
        
        if (machineCode.has_value()) {
            EncodedInstruction encoded;
            encoded.machineCode = machineCode.value();
            encoded.address = instr.address;
            encoded.sourceLine = instr.line;
            encoded.sourceText = instr.originalText;
            encoded.disassembly = isa::disassemble(machineCode.value());
            
            result.instructions.push_back(encoded);
            result.totalInstructions++;
        } else {
            // 编码失败，错误已经被记录
        }
    }
    
    // 设置结果
    result.success = errors.empty();
    result.errors = errors;
    result.warnings = warnings;
    
    if (!result.instructions.empty()) {
        result.textEndAddress = result.instructions.back().address;
    }
    
    return result;
}

std::optional<uint32_t> AsmEncoder::encodeInstruction(const ParsedInstruction& instr,
                                                       const SymbolTable& symbolTable) {
    // 获取指令类型
    isa::InstructionType type = isa::getInstructionType(instr.opcode);
    
    switch (type) {
        case isa::InstructionType::R_TYPE:
            return encodeRType(instr.opcode, instr.rd, instr.rs, instr.rt);
            
        case isa::InstructionType::I_TYPE: {
            int32_t immediate = instr.immediate;
            
            // 如果有标签引用，需要解析
            if (instr.hasLabelRef) {
                uint32_t targetAddr;
                if (!resolveLabel(instr.labelRef, symbolTable, targetAddr)) {
                    reportError(EncoderErrorType::UNRESOLVED_LABEL,
                               "未定义的标签: " + instr.labelRef,
                               instr.line, instr.address);
                    return std::nullopt;
                }
                
                // 对于分支指令，计算相对偏移
                const auto& info = isa::getInstructionInfo(instr.opcode);
                if (info.isBranch) {
                    immediate = calculateBranchOffset(instr.address, targetAddr);
                } else {
                    immediate = static_cast<int32_t>(targetAddr);
                }
            }
            
            // 验证立即数范围
            if (!validateImmediate(immediate)) {
                reportError(EncoderErrorType::IMMEDIATE_OUT_OF_RANGE,
                           "立即数超出范围: " + std::to_string(immediate) +
                           " (有效范围: -65536 到 65535)",
                           instr.line, instr.address);
                return std::nullopt;
            }
            
            return encodeIType(instr.opcode, instr.rd, instr.rs, immediate);
        }
            
        case isa::InstructionType::J_TYPE: {
            uint32_t target = instr.target;
            
            // 如果有标签引用，需要解析
            if (instr.hasLabelRef) {
                if (!resolveLabel(instr.labelRef, symbolTable, target)) {
                    reportError(EncoderErrorType::UNRESOLVED_LABEL,
                               "未定义的标签: " + instr.labelRef,
                               instr.line, instr.address);
                    return std::nullopt;
                }
            }
            
            // 验证目标地址范围
            if (!validateTarget(target)) {
                reportError(EncoderErrorType::TARGET_OUT_OF_RANGE,
                           "跳转目标超出范围: " + std::to_string(target) +
                           " (最大: 134217727)",
                           instr.line, instr.address);
                return std::nullopt;
            }
            
            return encodeJType(instr.opcode, target);
        }
            
        default:
            reportError(EncoderErrorType::INVALID_OPCODE,
                       "无效的操作码",
                       instr.line, instr.address);
            return std::nullopt;
    }
}

// ============================================================================
// 指令编码函数
// ============================================================================

uint32_t AsmEncoder::encodeRType(isa::Opcode opcode, isa::Register rd,
                                  isa::Register rs, isa::Register rt) {
    // R-type: opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12)
    // 位置：   [31:27]   [26:22] [21:17] [16:12] [11:0]
    
    uint32_t code = 0;
    
    // 编码 opcode（位 31:27）
    code |= (static_cast<uint32_t>(isa::opcodeValue(opcode)) & 0x1F) << isa::OPCODE_POS;
    
    // 编码 rd（位 26:22）
    code |= (static_cast<uint32_t>(isa::registerValue(rd)) & 0x1F) << isa::RD_POS;
    
    // 编码 rs（位 21:17）
    code |= (static_cast<uint32_t>(isa::registerValue(rs)) & 0x1F) << isa::RS_POS;
    
    // 编码 rt（位 16:12）
    code |= (static_cast<uint32_t>(isa::registerValue(rt)) & 0x1F) << isa::RT_POS;
    
    // 位 11:0 填充为 0（已经是 0）
    
    return code;
}

uint32_t AsmEncoder::encodeIType(isa::Opcode opcode, isa::Register rd,
                                  isa::Register rs, int32_t immediate) {
    // I-type: opcode(5) | rd(5) | rs(5) | immediate(17)
    // 位置：   [31:27]   [26:22] [21:17] [16:0]
    
    uint32_t code = 0;
    
    // 编码 opcode（位 31:27）
    code |= (static_cast<uint32_t>(isa::opcodeValue(opcode)) & 0x1F) << isa::OPCODE_POS;
    
    // 编码 rd（位 26:22）
    code |= (static_cast<uint32_t>(isa::registerValue(rd)) & 0x1F) << isa::RD_POS;
    
    // 编码 rs（位 21:17）
    code |= (static_cast<uint32_t>(isa::registerValue(rs)) & 0x1F) << isa::RS_POS;
    
    // 编码 immediate（位 16:0，17位有符号数）
    // 需要截取低17位
    code |= static_cast<uint32_t>(immediate) & isa::IMMEDIATE_MASK;
    
    return code;
}

uint32_t AsmEncoder::encodeJType(isa::Opcode opcode, uint32_t target) {
    // J-type: opcode(5) | target(27)
    // 位置：   [31:27]   [26:0]
    
    uint32_t code = 0;
    
    // 编码 opcode（位 31:27）
    code |= (static_cast<uint32_t>(isa::opcodeValue(opcode)) & 0x1F) << isa::OPCODE_POS;
    
    // 编码 target（位 26:0）
    code |= target & isa::TARGET_MASK;
    
    return code;
}

// ============================================================================
// 辅助函数
// ============================================================================

bool AsmEncoder::resolveLabel(const std::string& labelRef, const SymbolTable& symbolTable,
                               uint32_t& address) {
    auto it = symbolTable.find(labelRef);
    if (it == symbolTable.end()) {
        return false;
    }
    
    address = it->second.address;
    return true;
}

int32_t AsmEncoder::calculateBranchOffset(uint32_t currentAddr, uint32_t targetAddr) {
    // 分支偏移 = 目标地址 - (当前地址 + 1)
    // 因为 PC 在执行分支指令时已经指向下一条指令
    return static_cast<int32_t>(targetAddr) - static_cast<int32_t>(currentAddr + 1);
}

bool AsmEncoder::validateImmediate(int32_t immediate) {
    // 17位有符号数范围：-65536 到 65535
    return immediate >= isa::IMM_MIN && immediate <= isa::IMM_MAX;
}

bool AsmEncoder::validateTarget(uint32_t target) {
    // 27位无符号数范围：0 到 134217727
    return target <= isa::TARGET_MAX;
}

void AsmEncoder::reportError(EncoderErrorType type, const std::string& message,
                              int line, uint32_t address) {
    errors.emplace_back(type, message, line, address);
}

void AsmEncoder::reportWarning(const std::string& message) {
    warnings.push_back(message);
}

// ============================================================================
// 输出格式转换
// ============================================================================

std::string AsmEncoder::toBinaryString(uint32_t code) {
    std::bitset<32> bits(code);
    std::string result = bits.to_string();
    
    // 添加空格分隔，便于阅读
    // 格式：opcode(5) | rd(5) | rs(5) | rt/imm_high(5) | imm_low/zeros(12)
    std::string formatted;
    formatted += result.substr(0, 5) + " ";    // opcode
    formatted += result.substr(5, 5) + " ";    // rd
    formatted += result.substr(10, 5) + " ";   // rs
    formatted += result.substr(15, 5) + " ";   // rt or imm[16:12]
    formatted += result.substr(20, 12);        // zeros or imm[11:0]
    
    return formatted;
}

std::string AsmEncoder::toHexString(uint32_t code, bool prefix) {
    std::ostringstream oss;
    if (prefix) {
        oss << "0x";
    }
    oss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << code;
    return oss.str();
}

std::string AsmEncoder::disassembleFields(uint32_t code) {
    std::ostringstream oss;
    
    // 提取各字段
    isa::Opcode opcode = isa::extractOpcode(code);
    isa::Register rd = isa::extractRd(code);
    isa::Register rs = isa::extractRs(code);
    isa::Register rt = isa::extractRt(code);
    int32_t immediate = isa::extractImmediate(code);
    uint32_t target = isa::extractTarget(code);
    
    isa::InstructionType type = isa::getInstructionType(opcode);
    
    oss << "机器码: " << toHexString(code) << "\n";
    oss << "二进制: " << toBinaryString(code) << "\n";
    oss << "操作码: " << isa::opcodeToString(opcode) 
        << " (" << static_cast<int>(isa::opcodeValue(opcode)) << ")\n";
    
    switch (type) {
        case isa::InstructionType::R_TYPE:
            oss << "类型: R-type\n";
            oss << "rd: " << isa::registerToString(rd) 
                << " (" << static_cast<int>(isa::registerValue(rd)) << ")\n";
            oss << "rs: " << isa::registerToString(rs) 
                << " (" << static_cast<int>(isa::registerValue(rs)) << ")\n";
            oss << "rt: " << isa::registerToString(rt) 
                << " (" << static_cast<int>(isa::registerValue(rt)) << ")\n";
            break;
            
        case isa::InstructionType::I_TYPE:
            oss << "类型: I-type\n";
            oss << "rd: " << isa::registerToString(rd) 
                << " (" << static_cast<int>(isa::registerValue(rd)) << ")\n";
            oss << "rs: " << isa::registerToString(rs) 
                << " (" << static_cast<int>(isa::registerValue(rs)) << ")\n";
            oss << "immediate: " << immediate 
                << " (0x" << std::hex << (immediate & 0x1FFFF) << std::dec << ")\n";
            break;
            
        case isa::InstructionType::J_TYPE:
            oss << "类型: J-type\n";
            oss << "target: " << target 
                << " (0x" << std::hex << target << std::dec << ")\n";
            break;
            
        default:
            oss << "类型: 未知\n";
            break;
    }
    
    oss << "反汇编: " << isa::disassemble(code);
    
    return oss.str();
}

} // namespace assembler
