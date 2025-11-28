/**
 * @file Registers.hpp
 * @brief Custom RISC ISA Register Definitions
 * 
 * This file defines all 32 general-purpose registers ($r0-$r31).
 * 
 * Register Conventions:
 * - $r0: Always equals zero (constant 0)
 * - $r1 - $r30: General purpose registers
 * - $r31: Link register for jal instruction (stores return address)
 * 
 * Register Encoding: 5 bits (0-31)
 */

#ifndef ISA_REGISTERS_HPP
#define ISA_REGISTERS_HPP

#include <cstdint>
#include <string>

namespace isa {

/**
 * @brief Number of general-purpose registers
 */
constexpr uint8_t NUM_REGISTERS = 32;

/**
 * @brief Register width in bits
 */
constexpr uint8_t REGISTER_BITS = 32;

/**
 * @brief Register enumeration
 * 
 * Register conventions for this ISA:
 * - $r0:  Always 0, writes ignored
 * - $r1-$r30: General purpose
 * - $r31: Return address (link register for JAL)
 * 
 * We also provide MIPS-style aliases for convenience.
 */
enum class Register : uint8_t {
    // Numeric register names
    R0  = 0,
    R1  = 1,
    R2  = 2,
    R3  = 3,
    R4  = 4,
    R5  = 5,
    R6  = 6,
    R7  = 7,
    R8  = 8,
    R9  = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
    R16 = 16,
    R17 = 17,
    R18 = 18,
    R19 = 19,
    R20 = 20,
    R21 = 21,
    R22 = 22,
    R23 = 23,
    R24 = 24,
    R25 = 25,
    R26 = 26,
    R27 = 27,
    R28 = 28,
    R29 = 29,
    R30 = 30,
    R31 = 31,

    // Special aliases
    ZERO = 0,   // $r0 is always 0
    RA   = 31,  // Return address (link register)

    // MIPS-style aliases (optional, for convenience)
    AT   = 1,   // Assembler temporary
    V0   = 2,   // Return value
    V1   = 3,
    A0   = 4,   // Arguments
    A1   = 5,
    A2   = 6,
    A3   = 7,
    T0   = 8,   // Temporaries (caller-saved)
    T1   = 9,
    T2   = 10,
    T3   = 11,
    T4   = 12,
    T5   = 13,
    T6   = 14,
    T7   = 15,
    S0   = 16,  // Saved (callee-saved)
    S1   = 17,
    S2   = 18,
    S3   = 19,
    S4   = 20,
    S5   = 21,
    S6   = 22,
    S7   = 23,
    T8   = 24,
    T9   = 25,
    K0   = 26,  // Kernel reserved
    K1   = 27,
    GP   = 28,  // Global pointer
    SP   = 29,  // Stack pointer
    FP   = 30,  // Frame pointer

    // Invalid register marker
    INVALID = 0xFF
};

/**
 * @brief Get the numeric value of a register
 * @param reg The register
 * @return The 5-bit register number (0-31)
 */
inline uint8_t registerValue(Register reg) {
    return static_cast<uint8_t>(reg);
}

/**
 * @brief Check if a register number is valid
 * @param regNum The register number to check
 * @return true if valid (0-31), false otherwise
 */
inline bool isValidRegister(uint8_t regNum) {
    return regNum < NUM_REGISTERS;
}

/**
 * @brief Check if a register enum is valid
 * @param reg The register to check
 * @return true if valid, false otherwise
 */
inline bool isValidRegister(Register reg) {
    return reg != Register::INVALID && static_cast<uint8_t>(reg) < NUM_REGISTERS;
}

/**
 * @brief Convert register to string representation
 * 
 * @param reg The register to convert
 * @param useAlias If true, use MIPS-style names for common registers;
 *                 if false, always use $rN format
 * @return String representation of the register (e.g., "$r0", "$ra", "$t0")
 */
std::string registerToString(Register reg, bool useAlias = false);

/**
 * @brief Parse a register string to Register enum
 * 
 * Accepts formats:
 * - "$r0" to "$r31" (primary format)
 * - "$0" to "$31" (numeric without 'r')
 * - "r0" to "r31" (without '$')
 * - "$zero", "$ra", "$sp", etc. (MIPS-style aliases)
 * 
 * @param str The string to parse (case-insensitive)
 * @return The corresponding Register, or Register::INVALID if not found
 */
Register stringToRegister(const std::string& str);

/**
 * @brief Get a Register from a numeric value
 * @param num The register number (0-31)
 * @return The corresponding Register, or Register::INVALID if out of range
 */
inline Register registerFromNumber(uint8_t num) {
    return num < NUM_REGISTERS ? static_cast<Register>(num) : Register::INVALID;
}

/**
 * @brief Check if register is the zero register ($r0)
 * @param reg The register to check
 * @return true if $r0, false otherwise
 */
inline bool isZeroRegister(Register reg) {
    return reg == Register::R0;
}

/**
 * @brief Check if register is the link register ($r31)
 * @param reg The register to check
 * @return true if $r31, false otherwise
 */
inline bool isLinkRegister(Register reg) {
    return reg == Register::R31;
}

} // namespace isa

#endif // ISA_REGISTERS_HPP
