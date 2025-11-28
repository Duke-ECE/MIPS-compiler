/**
 * @file test_isa.cpp
 * @brief Unit tests for the ISA module
 * 
 * Tests cover:
 * - Opcode conversions and validation
 * - Register parsing and formatting
 * - Instruction encoding (R/I/J types)
 * - Instruction decoding
 * - Disassembly
 * - Round-trip encoding/decoding
 */

#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include "isa/Instructions.hpp"
#include <iostream>
#include <cassert>

using namespace isa;

// Test helper macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        std::cerr << "\nAssertion failed: " << #cond << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\nAssertion failed: " << #a << " != " << #b << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\nAssertion failed: " << #a << " != " << #b << std::endl; \
        std::cerr << "  Got: \"" << (a) << "\" Expected: \"" << (b) << "\"" << std::endl; \
        assert(false); \
    } \
} while(0)

// ============================================================================
// Opcode Tests
// ============================================================================

TEST(opcode_values) {
    // Verify opcode values match the ISA specification
    ASSERT_EQ(opcodeValue(Opcode::ADD),    0b00000);  // 0
    ASSERT_EQ(opcodeValue(Opcode::SUB),    0b00001);  // 1
    ASSERT_EQ(opcodeValue(Opcode::AND),    0b00010);  // 2
    ASSERT_EQ(opcodeValue(Opcode::OR),     0b00011);  // 3
    ASSERT_EQ(opcodeValue(Opcode::SLL),    0b00100);  // 4
    ASSERT_EQ(opcodeValue(Opcode::SRL),    0b00101);  // 5
    ASSERT_EQ(opcodeValue(Opcode::ADDI),   0b00110);  // 6
    ASSERT_EQ(opcodeValue(Opcode::LW),     0b00111);  // 7
    ASSERT_EQ(opcodeValue(Opcode::SW),     0b01000);  // 8
    ASSERT_EQ(opcodeValue(Opcode::BEQ),    0b01001);  // 9
    ASSERT_EQ(opcodeValue(Opcode::BGT),    0b01010);  // 10
    ASSERT_EQ(opcodeValue(Opcode::JR),     0b01011);  // 11
    ASSERT_EQ(opcodeValue(Opcode::J),      0b01100);  // 12
    ASSERT_EQ(opcodeValue(Opcode::JAL),    0b01101);  // 13
    ASSERT_EQ(opcodeValue(Opcode::INPUT),  0b01110);  // 14
    ASSERT_EQ(opcodeValue(Opcode::OUTPUT), 0b01111);  // 15
}

TEST(opcode_to_string) {
    ASSERT_STR_EQ(opcodeToString(Opcode::ADD), "add");
    ASSERT_STR_EQ(opcodeToString(Opcode::SUB), "sub");
    ASSERT_STR_EQ(opcodeToString(Opcode::AND), "and");
    ASSERT_STR_EQ(opcodeToString(Opcode::OR), "or");
    ASSERT_STR_EQ(opcodeToString(Opcode::SLL), "sll");
    ASSERT_STR_EQ(opcodeToString(Opcode::SRL), "srl");
    ASSERT_STR_EQ(opcodeToString(Opcode::ADDI), "addi");
    ASSERT_STR_EQ(opcodeToString(Opcode::LW), "lw");
    ASSERT_STR_EQ(opcodeToString(Opcode::SW), "sw");
    ASSERT_STR_EQ(opcodeToString(Opcode::BEQ), "beq");
    ASSERT_STR_EQ(opcodeToString(Opcode::BGT), "bgt");
    ASSERT_STR_EQ(opcodeToString(Opcode::JR), "jr");
    ASSERT_STR_EQ(opcodeToString(Opcode::J), "j");
    ASSERT_STR_EQ(opcodeToString(Opcode::JAL), "jal");
    ASSERT_STR_EQ(opcodeToString(Opcode::INPUT), "input");
    ASSERT_STR_EQ(opcodeToString(Opcode::OUTPUT), "output");
}

TEST(string_to_opcode) {
    ASSERT_EQ(stringToOpcode("add"), Opcode::ADD);
    ASSERT_EQ(stringToOpcode("ADD"), Opcode::ADD);
    ASSERT_EQ(stringToOpcode("AdD"), Opcode::ADD);
    ASSERT_EQ(stringToOpcode("lw"), Opcode::LW);
    ASSERT_EQ(stringToOpcode("beq"), Opcode::BEQ);
    ASSERT_EQ(stringToOpcode("jr"), Opcode::JR);
    ASSERT_EQ(stringToOpcode("jal"), Opcode::JAL);
    ASSERT_EQ(stringToOpcode("input"), Opcode::INPUT);
    ASSERT_EQ(stringToOpcode("output"), Opcode::OUTPUT);
    ASSERT_EQ(stringToOpcode("invalid"), Opcode::INVALID);
    ASSERT_EQ(stringToOpcode("xyz"), Opcode::INVALID);
}

TEST(instruction_type) {
    // R-type
    ASSERT_EQ(getInstructionType(Opcode::ADD), InstructionType::R_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::SUB), InstructionType::R_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::AND), InstructionType::R_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::OR), InstructionType::R_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::SLL), InstructionType::R_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::SRL), InstructionType::R_TYPE);
    
    // I-type
    ASSERT_EQ(getInstructionType(Opcode::ADDI), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::LW), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::SW), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::BEQ), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::BGT), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::JR), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::INPUT), InstructionType::I_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::OUTPUT), InstructionType::I_TYPE);
    
    // J-type
    ASSERT_EQ(getInstructionType(Opcode::J), InstructionType::J_TYPE);
    ASSERT_EQ(getInstructionType(Opcode::JAL), InstructionType::J_TYPE);
}

// ============================================================================
// Register Tests
// ============================================================================

TEST(register_to_string) {
    // Default format ($rN)
    ASSERT_STR_EQ(registerToString(Register::R0), "$r0");
    ASSERT_STR_EQ(registerToString(Register::R1), "$r1");
    ASSERT_STR_EQ(registerToString(Register::R31), "$r31");
    
    // With alias
    ASSERT_STR_EQ(registerToString(Register::ZERO, true), "$zero");
    ASSERT_STR_EQ(registerToString(Register::RA, true), "$ra");
    ASSERT_STR_EQ(registerToString(Register::SP, true), "$sp");
    ASSERT_STR_EQ(registerToString(Register::T0, true), "$t0");
}

TEST(string_to_register) {
    // $rN format
    ASSERT_EQ(stringToRegister("$r0"), Register::R0);
    ASSERT_EQ(stringToRegister("$r1"), Register::R1);
    ASSERT_EQ(stringToRegister("$r31"), Register::R31);
    
    // Without $
    ASSERT_EQ(stringToRegister("r0"), Register::R0);
    ASSERT_EQ(stringToRegister("r31"), Register::R31);
    
    // Numeric only
    ASSERT_EQ(stringToRegister("$0"), Register::R0);
    ASSERT_EQ(stringToRegister("$31"), Register::R31);
    
    // MIPS-style aliases
    ASSERT_EQ(stringToRegister("$zero"), Register::ZERO);
    ASSERT_EQ(stringToRegister("$ra"), Register::RA);
    ASSERT_EQ(stringToRegister("$sp"), Register::SP);
    ASSERT_EQ(stringToRegister("$t0"), Register::T0);
    
    // Case insensitive
    ASSERT_EQ(stringToRegister("$RA"), Register::RA);
    ASSERT_EQ(stringToRegister("$SP"), Register::SP);
    
    // Invalid
    ASSERT_EQ(stringToRegister("invalid"), Register::INVALID);
    ASSERT_EQ(stringToRegister("$r32"), Register::INVALID);
}

TEST(register_validity) {
    ASSERT_TRUE(isValidRegister(Register::R0));
    ASSERT_TRUE(isValidRegister(Register::R31));
    ASSERT_TRUE(!isValidRegister(Register::INVALID));
    
    ASSERT_TRUE(isValidRegister(static_cast<uint8_t>(0)));
    ASSERT_TRUE(isValidRegister(static_cast<uint8_t>(31)));
    ASSERT_TRUE(!isValidRegister(static_cast<uint8_t>(32)));
}

TEST(special_registers) {
    ASSERT_TRUE(isZeroRegister(Register::R0));
    ASSERT_TRUE(isZeroRegister(Register::ZERO));
    ASSERT_TRUE(!isZeroRegister(Register::R1));
    
    ASSERT_TRUE(isLinkRegister(Register::R31));
    ASSERT_TRUE(isLinkRegister(Register::RA));
    ASSERT_TRUE(!isLinkRegister(Register::R30));
}

// ============================================================================
// Encoding Tests
// ============================================================================

TEST(encode_r_type) {
    // add $r1, $r2, $r3
    // opcode=00000, rd=00001, rs=00010, rt=00011, zeros=000000000000
    RTypeInstruction add(Opcode::ADD, Register::R1, Register::R2, Register::R3);
    uint32_t encoded = encodeRType(add);
    
    // Verify bit fields
    ASSERT_EQ(extractOpcode(encoded), Opcode::ADD);
    ASSERT_EQ(extractRd(encoded), Register::R1);
    ASSERT_EQ(extractRs(encoded), Register::R2);
    ASSERT_EQ(extractRt(encoded), Register::R3);
    
    // Verify the actual encoding
    // 00000 | 00001 | 00010 | 00011 | 000000000000
    // = 0000 0000 0100 0100 0110 0000 0000 0000
    // = 0x00446000
    uint32_t expected = (0b00000 << 27) | (1 << 22) | (2 << 17) | (3 << 12);
    ASSERT_EQ(encoded, expected);
}

TEST(encode_r_type_shift) {
    // sll $r5, $r6, $r7
    RTypeInstruction sll(Opcode::SLL, Register::R5, Register::R6, Register::R7);
    uint32_t encoded = encodeRType(sll);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::SLL);
    ASSERT_EQ(extractRd(encoded), Register::R5);
    ASSERT_EQ(extractRs(encoded), Register::R6);
    ASSERT_EQ(extractRt(encoded), Register::R7);
}

TEST(encode_i_type) {
    // addi $r1, $r2, 100
    ITypeInstruction addi(Opcode::ADDI, Register::R1, Register::R2, 100);
    uint32_t encoded = encodeIType(addi);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::ADDI);
    ASSERT_EQ(extractRd(encoded), Register::R1);
    ASSERT_EQ(extractRs(encoded), Register::R2);
    ASSERT_EQ(extractImmediate(encoded), 100);
}

TEST(encode_i_type_negative) {
    // addi $r1, $r2, -50
    ITypeInstruction addi(Opcode::ADDI, Register::R1, Register::R2, -50);
    uint32_t encoded = encodeIType(addi);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::ADDI);
    ASSERT_EQ(extractImmediate(encoded), -50);
}

TEST(encode_i_type_memory) {
    // lw $r3, 8($r4)
    ITypeInstruction lw(Opcode::LW, Register::R3, Register::R4, 8);
    uint32_t encoded = encodeIType(lw);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::LW);
    ASSERT_EQ(extractRd(encoded), Register::R3);
    ASSERT_EQ(extractRs(encoded), Register::R4);
    ASSERT_EQ(extractImmediate(encoded), 8);
}

TEST(encode_i_type_branch) {
    // beq $r1, $r2, 10
    ITypeInstruction beq(Opcode::BEQ, Register::R1, Register::R2, 10);
    uint32_t encoded = encodeIType(beq);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::BEQ);
    ASSERT_EQ(extractRd(encoded), Register::R1);
    ASSERT_EQ(extractRs(encoded), Register::R2);
    ASSERT_EQ(extractImmediate(encoded), 10);
}

TEST(encode_i_type_jr) {
    // jr $r31
    ITypeInstruction jr(Opcode::JR, Register::R31);
    uint32_t encoded = encodeIType(jr);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::JR);
    ASSERT_EQ(extractRd(encoded), Register::R31);
}

TEST(encode_j_type) {
    // j 1024
    JTypeInstruction j(Opcode::J, 1024);
    uint32_t encoded = encodeJType(j);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::J);
    ASSERT_EQ(extractTarget(encoded), 1024u);
}

TEST(encode_j_type_jal) {
    // jal 2048
    JTypeInstruction jal(Opcode::JAL, 2048);
    uint32_t encoded = encodeJType(jal);
    
    ASSERT_EQ(extractOpcode(encoded), Opcode::JAL);
    ASSERT_EQ(extractTarget(encoded), 2048u);
}

// ============================================================================
// Decoding Tests
// ============================================================================

TEST(decode_r_type) {
    RTypeInstruction original(Opcode::SUB, Register::R10, Register::R11, Register::R12);
    uint32_t encoded = encodeRType(original);
    RTypeInstruction decoded = decodeRType(encoded);
    
    ASSERT_EQ(decoded.opcode, original.opcode);
    ASSERT_EQ(decoded.rd, original.rd);
    ASSERT_EQ(decoded.rs, original.rs);
    ASSERT_EQ(decoded.rt, original.rt);
}

TEST(decode_i_type) {
    ITypeInstruction original(Opcode::LW, Register::R5, Register::R29, -8);
    uint32_t encoded = encodeIType(original);
    ITypeInstruction decoded = decodeIType(encoded);
    
    ASSERT_EQ(decoded.opcode, original.opcode);
    ASSERT_EQ(decoded.rd, original.rd);
    ASSERT_EQ(decoded.rs, original.rs);
    ASSERT_EQ(decoded.immediate, original.immediate);
}

TEST(decode_j_type) {
    JTypeInstruction original(Opcode::JAL, 0x1234);
    uint32_t encoded = encodeJType(original);
    JTypeInstruction decoded = decodeJType(encoded);
    
    ASSERT_EQ(decoded.opcode, original.opcode);
    ASSERT_EQ(decoded.target, original.target);
}

TEST(decode_variant) {
    // Test auto-detection of instruction type
    RTypeInstruction r(Opcode::ADD, Register::R1, Register::R2, Register::R3);
    Instruction decoded_r = decode(encodeRType(r));
    ASSERT_TRUE(std::holds_alternative<RTypeInstruction>(decoded_r));
    
    ITypeInstruction i(Opcode::ADDI, Register::R1, Register::R2, 42);
    Instruction decoded_i = decode(encodeIType(i));
    ASSERT_TRUE(std::holds_alternative<ITypeInstruction>(decoded_i));
    
    JTypeInstruction j(Opcode::J, 100);
    Instruction decoded_j = decode(encodeJType(j));
    ASSERT_TRUE(std::holds_alternative<JTypeInstruction>(decoded_j));
}

// ============================================================================
// Disassembly Tests
// ============================================================================

TEST(disassemble_r_type) {
    std::string asm_str = disassemble(makeAdd(Register::R1, Register::R2, Register::R3));
    ASSERT_STR_EQ(asm_str, "add $r1, $r2, $r3");
    
    asm_str = disassemble(makeSub(Register::R10, Register::R11, Register::R12));
    ASSERT_STR_EQ(asm_str, "sub $r10, $r11, $r12");
    
    asm_str = disassemble(makeSll(Register::R5, Register::R6, Register::R7));
    ASSERT_STR_EQ(asm_str, "sll $r5, $r6, $r7");
}

TEST(disassemble_i_type) {
    std::string asm_str = disassemble(makeAddi(Register::R1, Register::R2, 100));
    ASSERT_STR_EQ(asm_str, "addi $r1, $r2, 100");
    
    asm_str = disassemble(makeLw(Register::R5, Register::R29, -8));
    ASSERT_STR_EQ(asm_str, "lw $r5, -8($r29)");
    
    asm_str = disassemble(makeSw(Register::R6, Register::R29, 4));
    ASSERT_STR_EQ(asm_str, "sw $r6, 4($r29)");
    
    asm_str = disassemble(makeBeq(Register::R1, Register::R2, 10));
    ASSERT_STR_EQ(asm_str, "beq $r1, $r2, 10");
    
    asm_str = disassemble(makeBgt(Register::R3, Register::R4, -5));
    ASSERT_STR_EQ(asm_str, "bgt $r3, $r4, -5");
}

TEST(disassemble_i_type_single_reg) {
    std::string asm_str = disassemble(makeJr(Register::R31));
    ASSERT_STR_EQ(asm_str, "jr $r31");
    
    asm_str = disassemble(makeInput(Register::R5));
    ASSERT_STR_EQ(asm_str, "input $r5");
    
    asm_str = disassemble(makeOutput(Register::R6));
    ASSERT_STR_EQ(asm_str, "output $r6");
}

TEST(disassemble_j_type) {
    std::string asm_str = disassemble(makeJ(1024));
    ASSERT_STR_EQ(asm_str, "j 1024");
    
    asm_str = disassemble(makeJal(2048));
    ASSERT_STR_EQ(asm_str, "jal 2048");
}

// ============================================================================
// Validation Tests
// ============================================================================

TEST(immediate_validation) {
    ASSERT_TRUE(isValidImmediate(0));
    ASSERT_TRUE(isValidImmediate(100));
    ASSERT_TRUE(isValidImmediate(-100));
    ASSERT_TRUE(isValidImmediate(65535));   // max positive
    ASSERT_TRUE(isValidImmediate(-65536));  // max negative
    ASSERT_TRUE(!isValidImmediate(65536));
    ASSERT_TRUE(!isValidImmediate(-65537));
}

TEST(target_validation) {
    ASSERT_TRUE(isValidTarget(0));
    ASSERT_TRUE(isValidTarget(1024));
    ASSERT_TRUE(isValidTarget(TARGET_MAX));
    ASSERT_TRUE(!isValidTarget(TARGET_MAX + 1));
}

// ============================================================================
// Round-trip Tests
// ============================================================================

TEST(roundtrip_r_type) {
    RTypeInstruction original(Opcode::AND, Register::R4, Register::R5, Register::R6);
    uint32_t encoded1 = encodeRType(original);
    RTypeInstruction decoded = decodeRType(encoded1);
    uint32_t encoded2 = encodeRType(decoded);
    ASSERT_EQ(encoded1, encoded2);
}

TEST(roundtrip_i_type) {
    ITypeInstruction original(Opcode::BEQ, Register::R10, Register::R11, -128);
    uint32_t encoded1 = encodeIType(original);
    ITypeInstruction decoded = decodeIType(encoded1);
    uint32_t encoded2 = encodeIType(decoded);
    ASSERT_EQ(encoded1, encoded2);
}

TEST(roundtrip_j_type) {
    JTypeInstruction original(Opcode::JAL, 0x7FFFFF);
    uint32_t encoded1 = encodeJType(original);
    JTypeInstruction decoded = decodeJType(encoded1);
    uint32_t encoded2 = encodeJType(decoded);
    ASSERT_EQ(encoded1, encoded2);
}

// ============================================================================
// Instruction Info Tests
// ============================================================================

TEST(instruction_info) {
    const auto& add_info = getInstructionInfo(Opcode::ADD);
    ASSERT_STR_EQ(add_info.mnemonic, "add");
    ASSERT_EQ(add_info.type, InstructionType::R_TYPE);
    ASSERT_TRUE(add_info.hasRd);
    ASSERT_TRUE(add_info.hasRs);
    ASSERT_TRUE(add_info.hasRt);
    ASSERT_TRUE(!add_info.hasImmediate);
    ASSERT_TRUE(!add_info.isBranch);
    ASSERT_TRUE(!add_info.isJump);
    
    const auto& beq_info = getInstructionInfo(Opcode::BEQ);
    ASSERT_STR_EQ(beq_info.mnemonic, "beq");
    ASSERT_TRUE(beq_info.isBranch);
    ASSERT_TRUE(!beq_info.isJump);
    
    const auto& j_info = getInstructionInfo(Opcode::J);
    ASSERT_STR_EQ(j_info.mnemonic, "j");
    ASSERT_TRUE(!j_info.isBranch);
    ASSERT_TRUE(j_info.isJump);
    
    const auto& lw_info = getInstructionInfo(Opcode::LW);
    ASSERT_TRUE(lw_info.isMemory);
    
    const auto& input_info = getInstructionInfo(Opcode::INPUT);
    ASSERT_TRUE(input_info.isIO);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "ISA Module Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Opcode tests
    std::cout << "\n--- Opcode Tests ---" << std::endl;
    RUN_TEST(opcode_values);
    RUN_TEST(opcode_to_string);
    RUN_TEST(string_to_opcode);
    RUN_TEST(instruction_type);
    
    // Register tests
    std::cout << "\n--- Register Tests ---" << std::endl;
    RUN_TEST(register_to_string);
    RUN_TEST(string_to_register);
    RUN_TEST(register_validity);
    RUN_TEST(special_registers);
    
    // Encoding tests
    std::cout << "\n--- Encoding Tests ---" << std::endl;
    RUN_TEST(encode_r_type);
    RUN_TEST(encode_r_type_shift);
    RUN_TEST(encode_i_type);
    RUN_TEST(encode_i_type_negative);
    RUN_TEST(encode_i_type_memory);
    RUN_TEST(encode_i_type_branch);
    RUN_TEST(encode_i_type_jr);
    RUN_TEST(encode_j_type);
    RUN_TEST(encode_j_type_jal);
    
    // Decoding tests
    std::cout << "\n--- Decoding Tests ---" << std::endl;
    RUN_TEST(decode_r_type);
    RUN_TEST(decode_i_type);
    RUN_TEST(decode_j_type);
    RUN_TEST(decode_variant);
    
    // Disassembly tests
    std::cout << "\n--- Disassembly Tests ---" << std::endl;
    RUN_TEST(disassemble_r_type);
    RUN_TEST(disassemble_i_type);
    RUN_TEST(disassemble_i_type_single_reg);
    RUN_TEST(disassemble_j_type);
    
    // Validation tests
    std::cout << "\n--- Validation Tests ---" << std::endl;
    RUN_TEST(immediate_validation);
    RUN_TEST(target_validation);
    
    // Round-trip tests
    std::cout << "\n--- Round-trip Tests ---" << std::endl;
    RUN_TEST(roundtrip_r_type);
    RUN_TEST(roundtrip_i_type);
    RUN_TEST(roundtrip_j_type);
    
    // Instruction info tests
    std::cout << "\n--- Instruction Info Tests ---" << std::endl;
    RUN_TEST(instruction_info);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "All tests PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
