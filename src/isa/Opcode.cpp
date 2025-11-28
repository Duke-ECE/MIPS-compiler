/**
 * @file Opcode.cpp
 * @brief Opcode utility function implementations
 */

#include "isa/Opcode.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace isa {

namespace {

// Static instruction type mapping
const std::unordered_map<Opcode, InstructionType> typeMap = {
    // R-type instructions
    {Opcode::ADD,   InstructionType::R_TYPE},
    {Opcode::SUB,   InstructionType::R_TYPE},
    {Opcode::AND,   InstructionType::R_TYPE},
    {Opcode::OR,    InstructionType::R_TYPE},
    {Opcode::SLL,   InstructionType::R_TYPE},
    {Opcode::SRL,   InstructionType::R_TYPE},
    
    // I-type instructions
    {Opcode::ADDI,  InstructionType::I_TYPE},
    {Opcode::LW,    InstructionType::I_TYPE},
    {Opcode::SW,    InstructionType::I_TYPE},
    {Opcode::BEQ,   InstructionType::I_TYPE},
    {Opcode::BGT,   InstructionType::I_TYPE},
    {Opcode::JR,    InstructionType::I_TYPE},
    {Opcode::INPUT, InstructionType::I_TYPE},
    {Opcode::OUTPUT,InstructionType::I_TYPE},
    
    // J-type instructions
    {Opcode::J,     InstructionType::J_TYPE},
    {Opcode::JAL,   InstructionType::J_TYPE},
};

// Static opcode name mapping
const std::unordered_map<Opcode, std::string> opcodeNames = {
    {Opcode::ADD,    "add"},
    {Opcode::SUB,    "sub"},
    {Opcode::AND,    "and"},
    {Opcode::OR,     "or"},
    {Opcode::SLL,    "sll"},
    {Opcode::SRL,    "srl"},
    {Opcode::ADDI,   "addi"},
    {Opcode::LW,     "lw"},
    {Opcode::SW,     "sw"},
    {Opcode::BEQ,    "beq"},
    {Opcode::BGT,    "bgt"},
    {Opcode::JR,     "jr"},
    {Opcode::J,      "j"},
    {Opcode::JAL,    "jal"},
    {Opcode::INPUT,  "input"},
    {Opcode::OUTPUT, "output"},
};

// Reverse mapping (string to opcode)
std::unordered_map<std::string, Opcode> buildNameToOpcodeMap() {
    std::unordered_map<std::string, Opcode> map;
    for (const auto& [op, name] : opcodeNames) {
        map[name] = op;
    }
    return map;
}

const std::unordered_map<std::string, Opcode> nameToOpcode = buildNameToOpcodeMap();

} // anonymous namespace

InstructionType getInstructionType(Opcode op) {
    auto it = typeMap.find(op);
    if (it != typeMap.end()) {
        return it->second;
    }
    return InstructionType::INVALID;
}

std::string opcodeToString(Opcode op) {
    auto it = opcodeNames.find(op);
    if (it != opcodeNames.end()) {
        return it->second;
    }
    return "unknown";
}

Opcode stringToOpcode(const std::string& str) {
    // Convert to lowercase for case-insensitive comparison
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    auto it = nameToOpcode.find(lower);
    if (it != nameToOpcode.end()) {
        return it->second;
    }
    return Opcode::INVALID;
}

bool isValidOpcode(Opcode op) {
    return typeMap.find(op) != typeMap.end();
}

} // namespace isa
