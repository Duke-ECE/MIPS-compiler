/**
 * @file Instructions.cpp
 * @brief Instruction encoding, decoding, and utility implementations
 */

#include "isa/Instructions.hpp"
#include <sstream>
#include <unordered_map>

namespace isa {

namespace {

// Static instruction info map
const std::unordered_map<Opcode, InstructionInfo> instructionInfoMap = {
    // R-type ALU instructions
    {Opcode::ADD, {
        Opcode::ADD, "add", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs + $rt",
        true, true, true, false, false, false, false, false, false
    }},
    {Opcode::SUB, {
        Opcode::SUB, "sub", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs - $rt",
        true, true, true, false, false, false, false, false, false
    }},
    {Opcode::AND, {
        Opcode::AND, "and", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs AND $rt",
        true, true, true, false, false, false, false, false, false
    }},
    {Opcode::OR, {
        Opcode::OR, "or", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs OR $rt",
        true, true, true, false, false, false, false, false, false
    }},
    {Opcode::SLL, {
        Opcode::SLL, "sll", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs << $rt[4:0] (zero-fill)",
        true, true, true, false, false, false, false, false, false
    }},
    {Opcode::SRL, {
        Opcode::SRL, "srl", InstructionType::R_TYPE,
        "$rd, $rs, $rt", "$rd = $rs >> $rt[4:0] (zero-extend)",
        true, true, true, false, false, false, false, false, false
    }},
    
    // I-type instructions
    {Opcode::ADDI, {
        Opcode::ADDI, "addi", InstructionType::I_TYPE,
        "$rd, $rs, N", "$rd = $rs + N",
        true, true, false, true, false, false, false, false, false
    }},
    {Opcode::LW, {
        Opcode::LW, "lw", InstructionType::I_TYPE,
        "$rd, N($rs)", "$rd = Mem[$rs + N]",
        true, true, false, true, false, false, false, true, false
    }},
    {Opcode::SW, {
        Opcode::SW, "sw", InstructionType::I_TYPE,
        "$rd, N($rs)", "Mem[$rs + N] = $rd",
        true, true, false, true, false, false, false, true, false
    }},
    {Opcode::BEQ, {
        Opcode::BEQ, "beq", InstructionType::I_TYPE,
        "$rd, $rs, N", "if ($rd == $rs) then PC = PC + 1 + N",
        true, true, false, true, false, true, false, false, false
    }},
    {Opcode::BGT, {
        Opcode::BGT, "bgt", InstructionType::I_TYPE,
        "$rd, $rs, N", "if ($rd > $rs) then PC = PC + 1 + N",
        true, true, false, true, false, true, false, false, false
    }},
    {Opcode::JR, {
        Opcode::JR, "jr", InstructionType::I_TYPE,
        "$rd", "PC = $rd",
        true, false, false, false, false, false, true, false, false
    }},
    
    // J-type instructions
    {Opcode::J, {
        Opcode::J, "j", InstructionType::J_TYPE,
        "N", "PC = N",
        false, false, false, false, true, false, true, false, false
    }},
    {Opcode::JAL, {
        Opcode::JAL, "jal", InstructionType::J_TYPE,
        "N", "$r31 = PC + 1; PC = N",
        false, false, false, false, true, false, true, false, false
    }},
    
    // I/O instructions
    {Opcode::INPUT, {
        Opcode::INPUT, "input", InstructionType::I_TYPE,
        "$rd", "$rd = keyboard input",
        true, false, false, false, false, false, false, false, true
    }},
    {Opcode::OUTPUT, {
        Opcode::OUTPUT, "output", InstructionType::I_TYPE,
        "$rd", "print character $rd[7:0] on LCD display",
        true, false, false, false, false, false, false, false, true
    }},
};

// Invalid instruction info for unknown opcodes
const InstructionInfo invalidInstructionInfo = {
    Opcode::INVALID, "unknown", InstructionType::INVALID,
    "", "Unknown instruction",
    false, false, false, false, false, false, false, false, false
};

} // anonymous namespace

// ============================================================================
// Instruction Info
// ============================================================================

const InstructionInfo& getInstructionInfo(Opcode op) {
    auto it = instructionInfoMap.find(op);
    if (it != instructionInfoMap.end()) {
        return it->second;
    }
    return invalidInstructionInfo;
}

// ============================================================================
// Encoding Functions
// ============================================================================

uint32_t encodeRType(const RTypeInstruction& instr) {
    uint32_t encoded = 0;
    
    // Format: opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12)
    encoded |= (static_cast<uint32_t>(instr.opcode) & 0x1F) << OPCODE_POS;
    encoded |= (static_cast<uint32_t>(instr.rd) & 0x1F) << RD_POS;
    encoded |= (static_cast<uint32_t>(instr.rs) & 0x1F) << RS_POS;
    encoded |= (static_cast<uint32_t>(instr.rt) & 0x1F) << RT_POS;
    // bits [11:0] are zeros
    
    return encoded;
}

uint32_t encodeIType(const ITypeInstruction& instr) {
    uint32_t encoded = 0;
    
    // Format: opcode(5) | rd(5) | rs(5) | immediate(17)
    encoded |= (static_cast<uint32_t>(instr.opcode) & 0x1F) << OPCODE_POS;
    encoded |= (static_cast<uint32_t>(instr.rd) & 0x1F) << RD_POS;
    encoded |= (static_cast<uint32_t>(instr.rs) & 0x1F) << RS_POS;
    // Mask immediate to 17 bits (handles negative numbers via 2's complement)
    encoded |= static_cast<uint32_t>(instr.immediate) & IMMEDIATE_MASK;
    
    return encoded;
}

uint32_t encodeJType(const JTypeInstruction& instr) {
    uint32_t encoded = 0;
    
    // Format: opcode(5) | target(27)
    encoded |= (static_cast<uint32_t>(instr.opcode) & 0x1F) << OPCODE_POS;
    encoded |= instr.target & TARGET_MASK;
    
    return encoded;
}

uint32_t encode(const Instruction& instr) {
    return std::visit([](const auto& i) -> uint32_t {
        using T = std::decay_t<decltype(i)>;
        if constexpr (std::is_same_v<T, RTypeInstruction>) {
            return encodeRType(i);
        } else if constexpr (std::is_same_v<T, ITypeInstruction>) {
            return encodeIType(i);
        } else if constexpr (std::is_same_v<T, JTypeInstruction>) {
            return encodeJType(i);
        }
        return 0;
    }, instr);
}

// ============================================================================
// Decoding Functions
// ============================================================================

RTypeInstruction decodeRType(uint32_t machineCode) {
    RTypeInstruction instr;
    instr.opcode = extractOpcode(machineCode);
    instr.rd = extractRd(machineCode);
    instr.rs = extractRs(machineCode);
    instr.rt = extractRt(machineCode);
    return instr;
}

ITypeInstruction decodeIType(uint32_t machineCode) {
    ITypeInstruction instr;
    instr.opcode = extractOpcode(machineCode);
    instr.rd = extractRd(machineCode);
    instr.rs = extractRs(machineCode);
    instr.immediate = extractImmediate(machineCode);
    return instr;
}

JTypeInstruction decodeJType(uint32_t machineCode) {
    JTypeInstruction instr;
    instr.opcode = extractOpcode(machineCode);
    instr.target = extractTarget(machineCode);
    return instr;
}

Instruction decode(uint32_t machineCode) {
    Opcode op = extractOpcode(machineCode);
    InstructionType type = getInstructionType(op);
    
    switch (type) {
        case InstructionType::R_TYPE:
            return decodeRType(machineCode);
        case InstructionType::I_TYPE:
            return decodeIType(machineCode);
        case InstructionType::J_TYPE:
            return decodeJType(machineCode);
        default:
            // Return a default R-type for invalid opcodes
            return RTypeInstruction();
    }
}

// ============================================================================
// Disassembly Functions
// ============================================================================

std::string disassemble(uint32_t machineCode) {
    return disassemble(decode(machineCode));
}

std::string disassemble(const Instruction& instr) {
    std::ostringstream oss;
    
    std::visit([&oss](const auto& i) {
        using T = std::decay_t<decltype(i)>;
        
        if constexpr (std::is_same_v<T, RTypeInstruction>) {
            const auto& info = getInstructionInfo(i.opcode);
            oss << info.mnemonic << " "
                << registerToString(i.rd) << ", "
                << registerToString(i.rs) << ", "
                << registerToString(i.rt);
                
        } else if constexpr (std::is_same_v<T, ITypeInstruction>) {
            const auto& info = getInstructionInfo(i.opcode);
            oss << info.mnemonic;
            
            if (i.opcode == Opcode::LW || i.opcode == Opcode::SW) {
                // Memory instructions: $rd, N($rs)
                oss << " " << registerToString(i.rd)
                    << ", " << i.immediate
                    << "(" << registerToString(i.rs) << ")";
            } else if (i.opcode == Opcode::JR || i.opcode == Opcode::INPUT || 
                       i.opcode == Opcode::OUTPUT) {
                // Single register instructions: $rd
                oss << " " << registerToString(i.rd);
            } else {
                // Standard I-type: $rd, $rs, N
                oss << " " << registerToString(i.rd)
                    << ", " << registerToString(i.rs)
                    << ", " << i.immediate;
            }
            
        } else if constexpr (std::is_same_v<T, JTypeInstruction>) {
            const auto& info = getInstructionInfo(i.opcode);
            oss << info.mnemonic << " " << i.target;
        }
    }, instr);
    
    return oss.str();
}

} // namespace isa
