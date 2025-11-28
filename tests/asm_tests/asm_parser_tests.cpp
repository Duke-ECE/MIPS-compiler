/**
 * @file asm_parser_tests.cpp
 * @brief AsmParser 单元测试
 * 
 * 测试覆盖：
 * - R-type 指令解析
 * - I-type 指令解析（包括 lw/sw 的内存操作数格式）
 * - J-type 指令解析
 * - 标签定义和引用
 * - 分支偏移计算
 * - 符号表构建
 * - 错误处理
 */

#include "assembler/asm_parser.hpp"
#include "assembler/asm_lexer.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace assembler;
using namespace isa;

// ============================================================================
// 测试辅助宏
// ============================================================================

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        std::cerr << "\n断言失败: " << #cond << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_FALSE(cond) do { \
    if ((cond)) { \
        std::cerr << "\n断言失败: NOT " << #cond << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\n断言失败: " << #a << " != " << #b << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\n断言失败: " << #a << " != " << #b << std::endl; \
        std::cerr << "  实际: \"" << (a) << "\" 期望: \"" << (b) << "\"" << std::endl; \
        assert(false); \
    } \
} while(0)

// ============================================================================
// R-type 指令测试
// ============================================================================

TEST(r_type_add) {
    std::string code = "add $r1, $r2, $r3";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::ADD);
    ASSERT_EQ(instr.type, InstructionType::R_TYPE);
    ASSERT_EQ(instr.rd, Register::R1);
    ASSERT_EQ(instr.rs, Register::R2);
    ASSERT_EQ(instr.rt, Register::R3);
}

TEST(r_type_sub) {
    std::string code = "sub $r4, $r5, $r6";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::SUB);
    ASSERT_EQ(instr.rd, Register::R4);
    ASSERT_EQ(instr.rs, Register::R5);
    ASSERT_EQ(instr.rt, Register::R6);
}

TEST(r_type_all_instructions) {
    std::vector<std::pair<std::string, Opcode>> tests = {
        {"add $r1, $r2, $r3", Opcode::ADD},
        {"sub $r1, $r2, $r3", Opcode::SUB},
        {"and $r1, $r2, $r3", Opcode::AND},
        {"or $r1, $r2, $r3", Opcode::OR},
        {"sll $r1, $r2, $r3", Opcode::SLL},
        {"srl $r1, $r2, $r3", Opcode::SRL},
    };
    
    for (const auto& [code, expectedOp] : tests) {
        AsmParser parser(code);
        ParseResult result = parser.parse();
        
        ASSERT_TRUE(result.success);
        ASSERT_EQ(result.instructions.size(), 1u);
        ASSERT_EQ(result.instructions[0].opcode, expectedOp);
    }
}

TEST(r_type_with_aliases) {
    std::string code = "add $t0, $zero, $ra";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.rd, Register::T0);
    ASSERT_EQ(instr.rs, Register::ZERO);
    ASSERT_EQ(instr.rt, Register::RA);
}

// ============================================================================
// I-type 指令测试
// ============================================================================

TEST(i_type_addi) {
    std::string code = "addi $r1, $r2, 100";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::ADDI);
    ASSERT_EQ(instr.type, InstructionType::I_TYPE);
    ASSERT_EQ(instr.rd, Register::R1);
    ASSERT_EQ(instr.rs, Register::R2);
    ASSERT_EQ(instr.immediate, 100);
}

TEST(i_type_addi_negative) {
    std::string code = "addi $r1, $r0, -50";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].immediate, -50);
}

TEST(i_type_lw) {
    std::string code = "lw $r1, 100($r2)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::LW);
    ASSERT_EQ(instr.rd, Register::R1);
    ASSERT_EQ(instr.rs, Register::R2);
    ASSERT_EQ(instr.immediate, 100);
}

TEST(i_type_sw) {
    std::string code = "sw $r3, 200($r4)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::SW);
    ASSERT_EQ(instr.rd, Register::R3);
    ASSERT_EQ(instr.rs, Register::R4);
    ASSERT_EQ(instr.immediate, 200);
}

TEST(i_type_lw_zero_offset) {
    std::string code = "lw $r1, 0($r2)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].immediate, 0);
}

TEST(i_type_lw_negative_offset) {
    std::string code = "lw $r1, -4($r2)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].immediate, -4);
}

TEST(i_type_beq_with_offset) {
    std::string code = "beq $r1, $r2, 10";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::BEQ);
    ASSERT_EQ(instr.rd, Register::R1);
    ASSERT_EQ(instr.rs, Register::R2);
    ASSERT_EQ(instr.immediate, 10);
}

TEST(i_type_jr) {
    std::string code = "jr $ra";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::JR);
    ASSERT_EQ(instr.rd, Register::RA);
}

TEST(i_type_input) {
    std::string code = "input $r1";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::INPUT);
    ASSERT_EQ(instr.rd, Register::R1);
}

TEST(i_type_output) {
    std::string code = "output $r2";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::OUTPUT);
    ASSERT_EQ(instr.rd, Register::R2);
}

// ============================================================================
// J-type 指令测试
// ============================================================================

TEST(j_type_j_with_address) {
    std::string code = "j 100";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::J);
    ASSERT_EQ(instr.type, InstructionType::J_TYPE);
    ASSERT_EQ(instr.target, 100u);
}

TEST(j_type_jal_with_address) {
    std::string code = "jal 200";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    const auto& instr = result.instructions[0];
    ASSERT_EQ(instr.opcode, Opcode::JAL);
    ASSERT_EQ(instr.target, 200u);
}

// ============================================================================
// 标签测试
// ============================================================================

TEST(label_definition) {
    std::string code = R"(
main:
    add $r1, $r2, $r3
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.symbols.size(), 1u);
    ASSERT_TRUE(result.symbols.find("main") != result.symbols.end());
    ASSERT_EQ(result.symbols.at("main").address, 0u);
}

TEST(multiple_labels) {
    std::string code = R"(
start:
    add $r1, $r2, $r3
middle:
    sub $r4, $r5, $r6
end:
    jr $ra
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.symbols.size(), 3u);
    ASSERT_EQ(result.symbols.at("start").address, 0u);
    ASSERT_EQ(result.symbols.at("middle").address, 1u);
    ASSERT_EQ(result.symbols.at("end").address, 2u);
}

TEST(j_type_with_label) {
    std::string code = R"(
start:
    add $r1, $r2, $r3
    j end
    sub $r4, $r5, $r6
end:
    jr $ra
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    // j end 应该解析为目标地址 3
    const auto& jInstr = result.instructions[1];
    ASSERT_EQ(jInstr.opcode, Opcode::J);
    ASSERT_EQ(jInstr.target, 3u); // end 标签的地址
}

TEST(beq_with_label) {
    std::string code = R"(
loop:
    addi $r1, $r1, -1
    beq $r1, $r0, done
    j loop
done:
    output $r1
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    // beq 指令在地址 1，done 在地址 3
    // 偏移量 = 3 - (1 + 1) = 1
    const auto& beqInstr = result.instructions[1];
    ASSERT_EQ(beqInstr.opcode, Opcode::BEQ);
    ASSERT_EQ(beqInstr.immediate, 1);
}

TEST(backward_branch) {
    std::string code = R"(
loop:
    addi $r1, $r1, -1
    bgt $r1, $r0, loop
    output $r1
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    // bgt 指令在地址 1，loop 在地址 0
    // 偏移量 = 0 - (1 + 1) = -2
    const auto& bgtInstr = result.instructions[1];
    ASSERT_EQ(bgtInstr.opcode, Opcode::BGT);
    ASSERT_EQ(bgtInstr.immediate, -2);
}

// ============================================================================
// 汇编指令测试
// ============================================================================

TEST(directive_text) {
    std::string code = R"(
.text
    add $r1, $r2, $r3
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 1u);
}

// ============================================================================
// 完整程序测试
// ============================================================================

TEST(complete_program) {
    std::string code = R"(
# 计算 1+2+...+10 的程序
.text
main:
    addi $r1, $r0, 0      # sum = 0
    addi $r2, $r0, 1      # i = 1
    addi $r3, $r0, 10     # limit = 10
loop:
    add $r1, $r1, $r2     # sum += i
    addi $r2, $r2, 1      # i++
    bgt $r3, $r2, loop    # if limit > i, goto loop
    output $r1            # 输出结果
    jr $ra                # 返回
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 8u);
    ASSERT_EQ(result.symbols.size(), 2u); // main, loop
    ASSERT_TRUE(result.symbols.find("main") != result.symbols.end());
    ASSERT_TRUE(result.symbols.find("loop") != result.symbols.end());
}

TEST(function_call) {
    std::string code = R"(
main:
    addi $r1, $r0, 5
    jal multiply
    output $r1
    jr $ra

multiply:
    add $r1, $r1, $r1
    jr $ra
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    
    // jal multiply 应该跳转到地址 4
    const auto& jalInstr = result.instructions[1];
    ASSERT_EQ(jalInstr.opcode, Opcode::JAL);
    ASSERT_EQ(jalInstr.target, 4u);
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST(error_undefined_label) {
    std::string code = "j undefined_label";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_FALSE(result.success);
    ASSERT_FALSE(result.errors.empty());
    ASSERT_EQ(result.errors[0].type, AsmErrorType::UNDEFINED_LABEL);
}

TEST(error_duplicate_label) {
    std::string code = R"(
label:
    add $r1, $r2, $r3
label:
    sub $r1, $r2, $r3
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_FALSE(result.success);
    bool foundDuplicateError = false;
    for (const auto& err : result.errors) {
        if (err.type == AsmErrorType::DUPLICATE_LABEL) {
            foundDuplicateError = true;
            break;
        }
    }
    ASSERT_TRUE(foundDuplicateError);
}

TEST(error_missing_operand) {
    std::string code = "add $r1, $r2";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_FALSE(result.success);
}

TEST(error_invalid_register) {
    std::string code = "add $r32, $r1, $r2";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_FALSE(result.success);
}

// ============================================================================
// 地址计算测试
// ============================================================================

TEST(instruction_addresses) {
    std::string code = R"(
    add $r1, $r2, $r3
    sub $r4, $r5, $r6
    and $r7, $r8, $r9
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 3u);
    ASSERT_EQ(result.instructions[0].address, 0u);
    ASSERT_EQ(result.instructions[1].address, 1u);
    ASSERT_EQ(result.instructions[2].address, 2u);
}

TEST(custom_text_start) {
    std::string code = R"(
main:
    add $r1, $r2, $r3
    j main
)";
    AsmParser parser(code);
    parser.setTextStart(100);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].address, 100u);
    ASSERT_EQ(result.instructions[1].address, 101u);
    ASSERT_EQ(result.instructions[1].target, 100u); // main 标签地址
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST(empty_program) {
    std::string code = "";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 0u);
}

TEST(only_labels) {
    std::string code = R"(
start:
middle:
end:
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions.size(), 0u);
    ASSERT_EQ(result.symbols.size(), 3u);
}

TEST(label_on_same_line) {
    // 标签和指令在同一行（标签单独处理）
    std::string code = R"(
start:
    add $r1, $r2, $r3
)";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.symbols.at("start").address, 0u);
}

TEST(hexadecimal_immediate) {
    std::string code = "addi $r1, $r0, 0xFF";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].immediate, 255);
}

TEST(binary_immediate) {
    std::string code = "addi $r1, $r0, 0b1010";
    AsmParser parser(code);
    ParseResult result = parser.parse();
    
    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.instructions[0].immediate, 10);
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "   AsmParser 单元测试" << std::endl;
    std::cout << "============================================" << std::endl;
    
    // R-type 测试
    RUN_TEST(r_type_add);
    RUN_TEST(r_type_sub);
    RUN_TEST(r_type_all_instructions);
    RUN_TEST(r_type_with_aliases);
    
    // I-type 测试
    RUN_TEST(i_type_addi);
    RUN_TEST(i_type_addi_negative);
    RUN_TEST(i_type_lw);
    RUN_TEST(i_type_sw);
    RUN_TEST(i_type_lw_zero_offset);
    RUN_TEST(i_type_lw_negative_offset);
    RUN_TEST(i_type_beq_with_offset);
    RUN_TEST(i_type_jr);
    RUN_TEST(i_type_input);
    RUN_TEST(i_type_output);
    
    // J-type 测试
    RUN_TEST(j_type_j_with_address);
    RUN_TEST(j_type_jal_with_address);
    
    // 标签测试
    RUN_TEST(label_definition);
    RUN_TEST(multiple_labels);
    RUN_TEST(j_type_with_label);
    RUN_TEST(beq_with_label);
    RUN_TEST(backward_branch);
    
    // 汇编指令测试
    RUN_TEST(directive_text);
    
    // 完整程序测试
    RUN_TEST(complete_program);
    RUN_TEST(function_call);
    
    // 错误处理测试
    RUN_TEST(error_undefined_label);
    RUN_TEST(error_duplicate_label);
    RUN_TEST(error_missing_operand);
    RUN_TEST(error_invalid_register);
    
    // 地址计算测试
    RUN_TEST(instruction_addresses);
    RUN_TEST(custom_text_start);
    
    // 边界情况测试
    RUN_TEST(empty_program);
    RUN_TEST(only_labels);
    RUN_TEST(label_on_same_line);
    RUN_TEST(hexadecimal_immediate);
    RUN_TEST(binary_immediate);
    
    std::cout << "============================================" << std::endl;
    std::cout << "   所有测试通过！" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
