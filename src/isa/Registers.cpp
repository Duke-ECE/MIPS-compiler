/**
 * @file Registers.cpp
 * @brief Register utility function implementations
 */

#include "isa/Registers.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace isa {

namespace {

// Build name-to-register mapping
std::unordered_map<std::string, Register> buildNameToRegisterMap() {
    std::unordered_map<std::string, Register> map;
    
    // Add $rN format (primary format for this ISA)
    for (uint8_t i = 0; i < NUM_REGISTERS; ++i) {
        map["$r" + std::to_string(i)] = static_cast<Register>(i);
        map["r" + std::to_string(i)] = static_cast<Register>(i);
        map["$" + std::to_string(i)] = static_cast<Register>(i);
        map[std::to_string(i)] = static_cast<Register>(i);
    }
    
    // Add MIPS-style aliases (optional convenience)
    map["$zero"] = Register::ZERO;
    map["zero"] = Register::ZERO;
    map["$ra"] = Register::RA;
    map["ra"] = Register::RA;
    map["$at"] = Register::AT;
    map["at"] = Register::AT;
    map["$v0"] = Register::V0;
    map["v0"] = Register::V0;
    map["$v1"] = Register::V1;
    map["v1"] = Register::V1;
    map["$a0"] = Register::A0;
    map["a0"] = Register::A0;
    map["$a1"] = Register::A1;
    map["a1"] = Register::A1;
    map["$a2"] = Register::A2;
    map["a2"] = Register::A2;
    map["$a3"] = Register::A3;
    map["a3"] = Register::A3;
    map["$t0"] = Register::T0;
    map["t0"] = Register::T0;
    map["$t1"] = Register::T1;
    map["t1"] = Register::T1;
    map["$t2"] = Register::T2;
    map["t2"] = Register::T2;
    map["$t3"] = Register::T3;
    map["t3"] = Register::T3;
    map["$t4"] = Register::T4;
    map["t4"] = Register::T4;
    map["$t5"] = Register::T5;
    map["t5"] = Register::T5;
    map["$t6"] = Register::T6;
    map["t6"] = Register::T6;
    map["$t7"] = Register::T7;
    map["t7"] = Register::T7;
    map["$t8"] = Register::T8;
    map["t8"] = Register::T8;
    map["$t9"] = Register::T9;
    map["t9"] = Register::T9;
    map["$s0"] = Register::S0;
    map["s0"] = Register::S0;
    map["$s1"] = Register::S1;
    map["s1"] = Register::S1;
    map["$s2"] = Register::S2;
    map["s2"] = Register::S2;
    map["$s3"] = Register::S3;
    map["s3"] = Register::S3;
    map["$s4"] = Register::S4;
    map["s4"] = Register::S4;
    map["$s5"] = Register::S5;
    map["s5"] = Register::S5;
    map["$s6"] = Register::S6;
    map["s6"] = Register::S6;
    map["$s7"] = Register::S7;
    map["s7"] = Register::S7;
    map["$k0"] = Register::K0;
    map["k0"] = Register::K0;
    map["$k1"] = Register::K1;
    map["k1"] = Register::K1;
    map["$gp"] = Register::GP;
    map["gp"] = Register::GP;
    map["$sp"] = Register::SP;
    map["sp"] = Register::SP;
    map["$fp"] = Register::FP;
    map["fp"] = Register::FP;
    
    return map;
}

const std::unordered_map<std::string, Register> nameToRegister = buildNameToRegisterMap();

} // anonymous namespace

std::string registerToString(Register reg, bool useAlias) {
    if (!isValidRegister(reg)) {
        return "invalid";
    }
    
    uint8_t regNum = static_cast<uint8_t>(reg);
    
    if (useAlias) {
        // Return MIPS-style alias for special registers
        switch (reg) {
            case Register::ZERO: return "$zero";
            case Register::AT:   return "$at";
            case Register::V0:   return "$v0";
            case Register::V1:   return "$v1";
            case Register::A0:   return "$a0";
            case Register::A1:   return "$a1";
            case Register::A2:   return "$a2";
            case Register::A3:   return "$a3";
            case Register::T0:   return "$t0";
            case Register::T1:   return "$t1";
            case Register::T2:   return "$t2";
            case Register::T3:   return "$t3";
            case Register::T4:   return "$t4";
            case Register::T5:   return "$t5";
            case Register::T6:   return "$t6";
            case Register::T7:   return "$t7";
            case Register::S0:   return "$s0";
            case Register::S1:   return "$s1";
            case Register::S2:   return "$s2";
            case Register::S3:   return "$s3";
            case Register::S4:   return "$s4";
            case Register::S5:   return "$s5";
            case Register::S6:   return "$s6";
            case Register::S7:   return "$s7";
            case Register::T8:   return "$t8";
            case Register::T9:   return "$t9";
            case Register::K0:   return "$k0";
            case Register::K1:   return "$k1";
            case Register::GP:   return "$gp";
            case Register::SP:   return "$sp";
            case Register::FP:   return "$fp";
            case Register::RA:   return "$ra";
            default: break;
        }
    }
    
    // Default: return $rN format
    return "$r" + std::to_string(regNum);
}

Register stringToRegister(const std::string& str) {
    if (str.empty()) {
        return Register::INVALID;
    }
    
    // Convert to lowercase for case-insensitive comparison
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    // Look up in the map
    auto it = nameToRegister.find(lower);
    if (it != nameToRegister.end()) {
        return it->second;
    }
    
    return Register::INVALID;
}

} // namespace isa
