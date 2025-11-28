/**
 * @file Opcode.hpp
 * @brief Custom RISC ISA Opcode Definitions
 * 
 * This file defines all opcode values for the custom RISC ISA.
 * The ISA supports R-type, I-type, and J-type instruction formats.
 * 
 * Instruction Format Overview:
 * - R-type: opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12)
 * - I-type: opcode(5) | rd(5) | rs(5) | immediate(17, signed)
 * - J-type: opcode(5) | target(27)
 * 
 * Note: Memory is word-addressed (each address = 32-bit word)
 */

#ifndef ISA_OPCODE_HPP
#define ISA_OPCODE_HPP

#include <cstdint>
#include <string>

namespace isa {

/**
 * @brief Opcode enumeration for all supported instructions
 * 
 * Opcode is 5 bits [31:27], allowing up to 32 different instructions.
 * Based on the ISA specification provided.
 */
enum class Opcode : uint8_t {
    // ============== R-type ALU Operations ==============
    ADD     = 0b00000,  // 0:  $rd = $rs + $rt
    SUB     = 0b00001,  // 1:  $rd = $rs - $rt
    AND     = 0b00010,  // 2:  $rd = $rs AND $rt
    OR      = 0b00011,  // 3:  $rd = $rs OR $rt
    SLL     = 0b00100,  // 4:  $rd = $rs shifted left by $rt[4:0], zero-fill
    SRL     = 0b00101,  // 5:  $rd = $rs shifted right by $rt[4:0], zero-extend

    // ============== I-type Operations ==============
    ADDI    = 0b00110,  // 6:  $rd = $rs + N (N is signed 17-bit immediate)
    LW      = 0b00111,  // 7:  $rd = Mem[$rs + N]
    SW      = 0b01000,  // 8:  Mem[$rs + N] = $rd
    BEQ     = 0b01001,  // 9:  if ($rd == $rs) then PC = PC + 1 + N
    BGT     = 0b01010,  // 10: if ($rd > $rs) then PC = PC + 1 + N
    JR      = 0b01011,  // 11: PC = $rd

    // ============== J-type Operations ==============
    J       = 0b01100,  // 12: PC = N (N is 27-bit target)
    JAL     = 0b01101,  // 13: $r31 = PC + 1; PC = N

    // ============== I/O Operations (I-type) ==============
    INPUT   = 0b01110,  // 14: $rd = keyboard input
    OUTPUT  = 0b01111,  // 15: print character $rd[7:0] on LCD display

    // Invalid opcode marker (not part of ISA)
    INVALID = 0xFF
};

/**
 * @brief Instruction format types
 */
enum class InstructionType {
    R_TYPE,     // Register-type: opcode rd rs rt zeros
    I_TYPE,     // Immediate-type: opcode rd rs immediate
    J_TYPE,     // Jump-type: opcode target
    INVALID
};

/**
 * @brief Get the instruction type for a given opcode
 * @param op The opcode to query
 * @return The instruction type
 */
InstructionType getInstructionType(Opcode op);

/**
 * @brief Convert opcode to string representation
 * @param op The opcode to convert
 * @return String name of the opcode (lowercase mnemonic)
 */
std::string opcodeToString(Opcode op);

/**
 * @brief Convert string to opcode
 * @param str The instruction mnemonic (case-insensitive)
 * @return The corresponding opcode, or Opcode::INVALID if not found
 */
Opcode stringToOpcode(const std::string& str);

/**
 * @brief Check if opcode is valid
 * @param op The opcode to check
 * @return true if valid, false otherwise
 */
bool isValidOpcode(Opcode op);

/**
 * @brief Get the numeric value of an opcode
 * @param op The opcode
 * @return The 5-bit opcode value
 */
inline uint8_t opcodeValue(Opcode op) {
    return static_cast<uint8_t>(op);
}

} // namespace isa

#endif // ISA_OPCODE_HPP
