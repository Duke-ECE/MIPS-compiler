/**
 * @file Instructions.hpp
 * @brief Custom RISC ISA Instruction Format Definitions
 * 
 * This file defines the instruction formats and encoding/decoding utilities.
 * 
 * Instruction Formats (32-bit):
 * 
 * R-type (Register):
 *   | opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12) |
 *   |31      27|26   22|21   17|16   12|11        0|
 * 
 * I-type (Immediate):
 *   | opcode(5) | rd(5) | rs(5) | immediate(17, signed) |
 *   |31      27|26   22|21   17|16                    0|
 * 
 * J-type (Jump):
 *   | opcode(5) | target(27) |
 *   |31      27|26         0|
 * 
 * Notes:
 * - Memory is word-addressed (each address = 32-bit word)
 * - Immediate field in I-type is 17-bit signed (2's complement)
 * - Unused register fields are filled with zeros
 * - $r0 always equals zero
 * - $r31 stores return address for JAL
 */

#ifndef ISA_INSTRUCTIONS_HPP
#define ISA_INSTRUCTIONS_HPP

#include "Opcode.hpp"
#include "Registers.hpp"
#include <cstdint>
#include <string>
#include <variant>

namespace isa {

// ============================================================================
// Bit Field Constants
// ============================================================================

// Instruction width
constexpr uint8_t INSTRUCTION_WIDTH = 32;

// Field widths
constexpr uint8_t OPCODE_WIDTH    = 5;
constexpr uint8_t REGISTER_WIDTH  = 5;
constexpr uint8_t ZEROS_WIDTH     = 12;  // R-type unused bits
constexpr uint8_t IMMEDIATE_WIDTH = 17;  // I-type immediate
constexpr uint8_t TARGET_WIDTH    = 27;  // J-type target

// Field positions (bit offset from LSB)
constexpr uint8_t OPCODE_POS      = 27;  // bits [31:27]
constexpr uint8_t RD_POS          = 22;  // bits [26:22]
constexpr uint8_t RS_POS          = 17;  // bits [21:17]
constexpr uint8_t RT_POS          = 12;  // bits [16:12]
constexpr uint8_t ZEROS_POS       = 0;   // bits [11:0]
constexpr uint8_t IMMEDIATE_POS   = 0;   // bits [16:0]
constexpr uint8_t TARGET_POS      = 0;   // bits [26:0]

// Field masks
constexpr uint32_t OPCODE_MASK    = 0x1F << OPCODE_POS;      // 0xF8000000
constexpr uint32_t RD_MASK        = 0x1F << RD_POS;          // 0x07C00000
constexpr uint32_t RS_MASK        = 0x1F << RS_POS;          // 0x003E0000
constexpr uint32_t RT_MASK        = 0x1F << RT_POS;          // 0x0001F000
constexpr uint32_t ZEROS_MASK     = 0xFFF;                   // 12 bits
constexpr uint32_t IMMEDIATE_MASK = 0x1FFFF;                 // 17 bits
constexpr uint32_t TARGET_MASK    = 0x07FFFFFF;              // 27 bits

// Immediate value range (17-bit signed: -65536 to +65535)
constexpr int32_t IMM_MIN = -(1 << 16);    // -65536
constexpr int32_t IMM_MAX = (1 << 16) - 1; // +65535

// Target address range (27-bit unsigned)
constexpr uint32_t TARGET_MAX = (1 << 27) - 1;  // 134217727

// ============================================================================
// Instruction Structures
// ============================================================================

/**
 * @brief R-type instruction structure
 * 
 * Format: opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12)
 * Used for: add, sub, and, or, sll, srl
 */
struct RTypeInstruction {
    Opcode opcode;
    Register rd;        // Destination register
    Register rs;        // Source register 1
    Register rt;        // Source register 2 (also shift amount for sll/srl)

    RTypeInstruction() 
        : opcode(Opcode::ADD), rd(Register::R0), rs(Register::R0), rt(Register::R0) {}
    
    RTypeInstruction(Opcode op, Register d, Register s, Register t)
        : opcode(op), rd(d), rs(s), rt(t) {}
};

/**
 * @brief I-type instruction structure
 * 
 * Format: opcode(5) | rd(5) | rs(5) | immediate(17)
 * Used for: addi, lw, sw, beq, bgt, jr, input, output
 * 
 * Note: For jr, only rd is used (PC = $rd)
 *       For input/output, only rd is used
 */
struct ITypeInstruction {
    Opcode opcode;
    Register rd;        // Destination/source register
    Register rs;        // Base register (for lw/sw/addi) or comparison (for beq/bgt)
    int32_t immediate;  // 17-bit signed immediate

    ITypeInstruction() 
        : opcode(Opcode::ADDI), rd(Register::R0), rs(Register::R0), immediate(0) {}
    
    ITypeInstruction(Opcode op, Register d, Register s, int32_t imm)
        : opcode(op), rd(d), rs(s), immediate(imm) {}
    
    // Constructor for instructions that only use rd (jr, input, output)
    ITypeInstruction(Opcode op, Register d)
        : opcode(op), rd(d), rs(Register::R0), immediate(0) {}
};

/**
 * @brief J-type instruction structure
 * 
 * Format: opcode(5) | target(27)
 * Used for: j, jal
 */
struct JTypeInstruction {
    Opcode opcode;
    uint32_t target;    // 27-bit target address

    JTypeInstruction() : opcode(Opcode::J), target(0) {}
    
    JTypeInstruction(Opcode op, uint32_t tgt)
        : opcode(op), target(tgt) {}
};

/**
 * @brief Unified instruction type using std::variant
 */
using Instruction = std::variant<RTypeInstruction, ITypeInstruction, JTypeInstruction>;

// ============================================================================
// Instruction Information
// ============================================================================

/**
 * @brief Detailed information about an instruction
 */
struct InstructionInfo {
    Opcode opcode;
    std::string mnemonic;       // e.g., "add", "lw", "beq"
    InstructionType type;       // R, I, or J
    std::string syntax;         // e.g., "$rd, $rs, $rt" or "$rd, N($rs)"
    std::string description;    // Human-readable description
    bool hasRd;                 // Uses rd field
    bool hasRs;                 // Uses rs field
    bool hasRt;                 // Uses rt field (R-type only)
    bool hasImmediate;          // Uses immediate field (I-type)
    bool hasTarget;             // Uses target field (J-type)
    bool isBranch;              // Is a branch instruction (beq, bgt)
    bool isJump;                // Is a jump instruction (j, jal, jr)
    bool isMemory;              // Is a memory operation (lw, sw)
    bool isIO;                  // Is an I/O operation (input, output)
};

/**
 * @brief Get instruction info for an opcode
 * @param op The opcode to query
 * @return InstructionInfo structure
 */
const InstructionInfo& getInstructionInfo(Opcode op);

// ============================================================================
// Encoding Functions
// ============================================================================

/**
 * @brief Encode an R-type instruction to 32-bit machine code
 * @param instr The R-type instruction
 * @return 32-bit encoded instruction
 */
uint32_t encodeRType(const RTypeInstruction& instr);

/**
 * @brief Encode an I-type instruction to 32-bit machine code
 * @param instr The I-type instruction
 * @return 32-bit encoded instruction
 */
uint32_t encodeIType(const ITypeInstruction& instr);

/**
 * @brief Encode a J-type instruction to 32-bit machine code
 * @param instr The J-type instruction
 * @return 32-bit encoded instruction
 */
uint32_t encodeJType(const JTypeInstruction& instr);

/**
 * @brief Encode any instruction variant to 32-bit machine code
 * @param instr The instruction variant
 * @return 32-bit encoded instruction
 */
uint32_t encode(const Instruction& instr);

// ============================================================================
// Decoding Functions
// ============================================================================

/**
 * @brief Decode a 32-bit machine code to instruction variant
 * @param machineCode The 32-bit encoded instruction
 * @return Decoded instruction variant
 */
Instruction decode(uint32_t machineCode);

/**
 * @brief Decode and return R-type instruction (assumes R-type)
 * @param machineCode The 32-bit encoded instruction
 * @return Decoded R-type instruction
 */
RTypeInstruction decodeRType(uint32_t machineCode);

/**
 * @brief Decode and return I-type instruction (assumes I-type)
 * @param machineCode The 32-bit encoded instruction
 * @return Decoded I-type instruction
 */
ITypeInstruction decodeIType(uint32_t machineCode);

/**
 * @brief Decode and return J-type instruction (assumes J-type)
 * @param machineCode The 32-bit encoded instruction
 * @return Decoded J-type instruction
 */
JTypeInstruction decodeJType(uint32_t machineCode);

// ============================================================================
// Field Extraction Helpers
// ============================================================================

/**
 * @brief Extract opcode from machine code
 */
inline Opcode extractOpcode(uint32_t machineCode) {
    return static_cast<Opcode>((machineCode >> OPCODE_POS) & 0x1F);
}

/**
 * @brief Extract rd field from machine code
 */
inline Register extractRd(uint32_t machineCode) {
    return static_cast<Register>((machineCode >> RD_POS) & 0x1F);
}

/**
 * @brief Extract rs field from machine code
 */
inline Register extractRs(uint32_t machineCode) {
    return static_cast<Register>((machineCode >> RS_POS) & 0x1F);
}

/**
 * @brief Extract rt field from machine code
 */
inline Register extractRt(uint32_t machineCode) {
    return static_cast<Register>((machineCode >> RT_POS) & 0x1F);
}

/**
 * @brief Extract immediate field from machine code (sign-extended to 32 bits)
 */
inline int32_t extractImmediate(uint32_t machineCode) {
    int32_t imm = machineCode & IMMEDIATE_MASK;
    // Sign extend from 17 bits to 32 bits
    if (imm & (1 << 16)) {
        imm |= 0xFFFE0000;  // Sign extend
    }
    return imm;
}

/**
 * @brief Extract target field from machine code
 */
inline uint32_t extractTarget(uint32_t machineCode) {
    return machineCode & TARGET_MASK;
}

// ============================================================================
// Validation Functions
// ============================================================================

/**
 * @brief Check if an immediate value fits in 17 bits (signed)
 * @param imm The immediate value to check
 * @return true if valid, false otherwise
 */
inline bool isValidImmediate(int32_t imm) {
    return imm >= IMM_MIN && imm <= IMM_MAX;
}

/**
 * @brief Check if a target address fits in 27 bits
 * @param target The target address to check
 * @return true if valid, false otherwise
 */
inline bool isValidTarget(uint32_t target) {
    return target <= TARGET_MAX;
}

// ============================================================================
// Disassembly Functions
// ============================================================================

/**
 * @brief Disassemble a 32-bit machine code to assembly string
 * @param machineCode The 32-bit encoded instruction
 * @return Assembly language string representation
 */
std::string disassemble(uint32_t machineCode);

/**
 * @brief Disassemble an instruction variant to assembly string
 * @param instr The instruction variant
 * @return Assembly language string representation
 */
std::string disassemble(const Instruction& instr);

// ============================================================================
// Instruction Builder Helpers
// ============================================================================

// R-type builders
inline Instruction makeAdd(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::ADD, rd, rs, rt);
}

inline Instruction makeSub(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::SUB, rd, rs, rt);
}

inline Instruction makeAnd(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::AND, rd, rs, rt);
}

inline Instruction makeOr(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::OR, rd, rs, rt);
}

inline Instruction makeSll(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::SLL, rd, rs, rt);
}

inline Instruction makeSrl(Register rd, Register rs, Register rt) {
    return RTypeInstruction(Opcode::SRL, rd, rs, rt);
}

// I-type builders
inline Instruction makeAddi(Register rd, Register rs, int32_t imm) {
    return ITypeInstruction(Opcode::ADDI, rd, rs, imm);
}

inline Instruction makeLw(Register rd, Register rs, int32_t offset) {
    return ITypeInstruction(Opcode::LW, rd, rs, offset);
}

inline Instruction makeSw(Register rd, Register rs, int32_t offset) {
    return ITypeInstruction(Opcode::SW, rd, rs, offset);
}

inline Instruction makeBeq(Register rd, Register rs, int32_t offset) {
    return ITypeInstruction(Opcode::BEQ, rd, rs, offset);
}

inline Instruction makeBgt(Register rd, Register rs, int32_t offset) {
    return ITypeInstruction(Opcode::BGT, rd, rs, offset);
}

inline Instruction makeJr(Register rd) {
    return ITypeInstruction(Opcode::JR, rd);
}

inline Instruction makeInput(Register rd) {
    return ITypeInstruction(Opcode::INPUT, rd);
}

inline Instruction makeOutput(Register rd) {
    return ITypeInstruction(Opcode::OUTPUT, rd);
}

// J-type builders
inline Instruction makeJ(uint32_t target) {
    return JTypeInstruction(Opcode::J, target);
}

inline Instruction makeJal(uint32_t target) {
    return JTypeInstruction(Opcode::JAL, target);
}

} // namespace isa

#endif // ISA_INSTRUCTIONS_HPP
