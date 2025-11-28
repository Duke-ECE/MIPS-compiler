/**
 * @file asm_encoder_tests.cpp
 * @brief AsmEncoder 单元测试
 * 
 * 测试覆盖：
 * - R-type 指令编码
 * - I-type 指令编码
 * - J-type 指令编码
 * - 立即数范围验证
 * - 标签解析
 * - 完整程序编码
 * - 输出格式转换
 */

#include "assembler/asm_encoder.hpp"
#include "assembler/asm_parser.hpp"
#include "isa/Instructions.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <iostream>
#include <cassert>
#include <cstdint>

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

#define ASSERT_EQ_HEX(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\n断言失败: " << #a << " != " << #b << std::endl; \
        std::cerr << std::hex << "  实际值: 0x" << (a) << ", 期望值: 0x" << (b) << std::dec << std::endl; \
        assert(false); \
    } \
} while(0)

// ============================================================================
// R-type 编码测试
// ============================================================================

TEST(encode_r_type_add) {
    AsmEncoder encoder;
    
    // add $r1, $r2, $r3
    // opcode=0 (00000), rd=1 (00001), rs=2 (00010), rt=3 (00011), zeros=0
    // 00000 00001 00010 00011 000000000000
    // = 0x00443000
    uint32_t code = encoder.encodeRType(Opcode::ADD, Register::R1, Register::R2, Register::R3);
    
    // 验证各字段
    ASSERT_EQ(extractOpcode(code), Opcode::ADD);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R2);
    ASSERT_EQ(extractRt(code), Register::R3);
}

TEST(encode_r_type_sub) {
    AsmEncoder encoder;
    
    // sub $r4, $r5, $r6
    // opcode=1 (00001), rd=4 (00100), rs=5 (00101), rt=6 (00110)
    uint32_t code = encoder.encodeRType(Opcode::SUB, Register::R4, Register::R5, Register::R6);
    
    ASSERT_EQ(extractOpcode(code), Opcode::SUB);
    ASSERT_EQ(extractRd(code), Register::R4);
    ASSERT_EQ(extractRs(code), Register::R5);
    ASSERT_EQ(extractRt(code), Register::R6);
}

TEST(encode_r_type_and) {
    AsmEncoder encoder;
    
    // and $r7, $r8, $r9
    uint32_t code = encoder.encodeRType(Opcode::AND, Register::R7, Register::R8, Register::R9);
    
    ASSERT_EQ(extractOpcode(code), Opcode::AND);
    ASSERT_EQ(extractRd(code), Register::R7);
    ASSERT_EQ(extractRs(code), Register::R8);
    ASSERT_EQ(extractRt(code), Register::R9);
}

TEST(encode_r_type_or) {
    AsmEncoder encoder;
    
    // or $r10, $r11, $r12
    uint32_t code = encoder.encodeRType(Opcode::OR, Register::R10, Register::R11, Register::R12);
    
    ASSERT_EQ(extractOpcode(code), Opcode::OR);
    ASSERT_EQ(extractRd(code), Register::R10);
    ASSERT_EQ(extractRs(code), Register::R11);
    ASSERT_EQ(extractRt(code), Register::R12);
}

TEST(encode_r_type_sll) {
    AsmEncoder encoder;
    
    // sll $r1, $r2, $r3
    uint32_t code = encoder.encodeRType(Opcode::SLL, Register::R1, Register::R2, Register::R3);
    
    ASSERT_EQ(extractOpcode(code), Opcode::SLL);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R2);
    ASSERT_EQ(extractRt(code), Register::R3);
}

TEST(encode_r_type_srl) {
    AsmEncoder encoder;
    
    // srl $r1, $r2, $r3
    uint32_t code = encoder.encodeRType(Opcode::SRL, Register::R1, Register::R2, Register::R3);
    
    ASSERT_EQ(extractOpcode(code), Opcode::SRL);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R2);
    ASSERT_EQ(extractRt(code), Register::R3);
}

TEST(encode_r_type_all_registers) {
    AsmEncoder encoder;
    
    // 测试所有 32 个寄存器
    for (int rd = 0; rd < 32; rd++) {
        for (int rs = 0; rs < 32; rs += 8) {  // 采样测试
            for (int rt = 0; rt < 32; rt += 8) {
                uint32_t code = encoder.encodeRType(
                    Opcode::ADD,
                    registerFromNumber(rd),
                    registerFromNumber(rs),
                    registerFromNumber(rt)
                );
                
                ASSERT_EQ(static_cast<int>(registerValue(extractRd(code))), rd);
                ASSERT_EQ(static_cast<int>(registerValue(extractRs(code))), rs);
                ASSERT_EQ(static_cast<int>(registerValue(extractRt(code))), rt);
            }
        }
    }
}

// ============================================================================
// I-type 编码测试
// ============================================================================

TEST(encode_i_type_addi) {
    AsmEncoder encoder;
    
    // addi $r1, $r0, 10
    // opcode=6 (00110), rd=1 (00001), rs=0 (00000), imm=10
    uint32_t code = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, 10);
    
    ASSERT_EQ(extractOpcode(code), Opcode::ADDI);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R0);
    ASSERT_EQ(extractImmediate(code), 10);
}

TEST(encode_i_type_addi_negative) {
    AsmEncoder encoder;
    
    // addi $r1, $r2, -100
    uint32_t code = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R2, -100);
    
    ASSERT_EQ(extractOpcode(code), Opcode::ADDI);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R2);
    ASSERT_EQ(extractImmediate(code), -100);
}

TEST(encode_i_type_lw) {
    AsmEncoder encoder;
    
    // lw $r4, 100($r5)
    // rd=4, rs=5, offset=100
    uint32_t code = encoder.encodeIType(Opcode::LW, Register::R4, Register::R5, 100);
    
    ASSERT_EQ(extractOpcode(code), Opcode::LW);
    ASSERT_EQ(extractRd(code), Register::R4);
    ASSERT_EQ(extractRs(code), Register::R5);
    ASSERT_EQ(extractImmediate(code), 100);
}

TEST(encode_i_type_sw) {
    AsmEncoder encoder;
    
    // sw $r6, 200($r7)
    uint32_t code = encoder.encodeIType(Opcode::SW, Register::R6, Register::R7, 200);
    
    ASSERT_EQ(extractOpcode(code), Opcode::SW);
    ASSERT_EQ(extractRd(code), Register::R6);
    ASSERT_EQ(extractRs(code), Register::R7);
    ASSERT_EQ(extractImmediate(code), 200);
}

TEST(encode_i_type_beq) {
    AsmEncoder encoder;
    
    // beq $r1, $r2, 5 (offset)
    uint32_t code = encoder.encodeIType(Opcode::BEQ, Register::R1, Register::R2, 5);
    
    ASSERT_EQ(extractOpcode(code), Opcode::BEQ);
    ASSERT_EQ(extractRd(code), Register::R1);
    ASSERT_EQ(extractRs(code), Register::R2);
    ASSERT_EQ(extractImmediate(code), 5);
}

TEST(encode_i_type_bgt) {
    AsmEncoder encoder;
    
    // bgt $r3, $r4, -3 (backward branch)
    uint32_t code = encoder.encodeIType(Opcode::BGT, Register::R3, Register::R4, -3);
    
    ASSERT_EQ(extractOpcode(code), Opcode::BGT);
    ASSERT_EQ(extractRd(code), Register::R3);
    ASSERT_EQ(extractRs(code), Register::R4);
    ASSERT_EQ(extractImmediate(code), -3);
}

TEST(encode_i_type_jr) {
    AsmEncoder encoder;
    
    // jr $r31 (return)
    uint32_t code = encoder.encodeIType(Opcode::JR, Register::R31, Register::R0, 0);
    
    ASSERT_EQ(extractOpcode(code), Opcode::JR);
    ASSERT_EQ(extractRd(code), Register::R31);
}

TEST(encode_i_type_input) {
    AsmEncoder encoder;
    
    // input $r1
    uint32_t code = encoder.encodeIType(Opcode::INPUT, Register::R1, Register::R0, 0);
    
    ASSERT_EQ(extractOpcode(code), Opcode::INPUT);
    ASSERT_EQ(extractRd(code), Register::R1);
}

TEST(encode_i_type_output) {
    AsmEncoder encoder;
    
    // output $r2
    uint32_t code = encoder.encodeIType(Opcode::OUTPUT, Register::R2, Register::R0, 0);
    
    ASSERT_EQ(extractOpcode(code), Opcode::OUTPUT);
    ASSERT_EQ(extractRd(code), Register::R2);
}

TEST(encode_i_type_immediate_range) {
    AsmEncoder encoder;
    
    // 测试立即数边界值
    // 最大正值：65535
    uint32_t code1 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, 65535);
    ASSERT_EQ(extractImmediate(code1), 65535);
    
    // 最小负值：-65536
    uint32_t code2 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, -65536);
    ASSERT_EQ(extractImmediate(code2), -65536);
    
    // 0
    uint32_t code3 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, 0);
    ASSERT_EQ(extractImmediate(code3), 0);
    
    // -1
    uint32_t code4 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, -1);
    ASSERT_EQ(extractImmediate(code4), -1);
}

// ============================================================================
// J-type 编码测试
// ============================================================================

TEST(encode_j_type_j) {
    AsmEncoder encoder;
    
    // j 1000
    uint32_t code = encoder.encodeJType(Opcode::J, 1000);
    
    ASSERT_EQ(extractOpcode(code), Opcode::J);
    ASSERT_EQ(extractTarget(code), 1000u);
}

TEST(encode_j_type_jal) {
    AsmEncoder encoder;
    
    // jal 2000
    uint32_t code = encoder.encodeJType(Opcode::JAL, 2000);
    
    ASSERT_EQ(extractOpcode(code), Opcode::JAL);
    ASSERT_EQ(extractTarget(code), 2000u);
}

TEST(encode_j_type_target_range) {
    AsmEncoder encoder;
    
    // 测试目标地址边界值
    // 最大值：134217727 (2^27 - 1)
    uint32_t code1 = encoder.encodeJType(Opcode::J, 134217727);
    ASSERT_EQ(extractTarget(code1), 134217727u);
    
    // 0
    uint32_t code2 = encoder.encodeJType(Opcode::J, 0);
    ASSERT_EQ(extractTarget(code2), 0u);
}

// ============================================================================
// 编码解码往返测试
// ============================================================================

TEST(roundtrip_r_type) {
    AsmEncoder encoder;
    
    // 编码
    uint32_t code = encoder.encodeRType(Opcode::ADD, Register::R10, Register::R20, Register::R30);
    
    // 解码
    RTypeInstruction decoded = decodeRType(code);
    
    // 验证往返
    ASSERT_EQ(decoded.opcode, Opcode::ADD);
    ASSERT_EQ(decoded.rd, Register::R10);
    ASSERT_EQ(decoded.rs, Register::R20);
    ASSERT_EQ(decoded.rt, Register::R30);
}

TEST(roundtrip_i_type) {
    AsmEncoder encoder;
    
    // 编码
    uint32_t code = encoder.encodeIType(Opcode::ADDI, Register::R5, Register::R10, -12345);
    
    // 解码
    ITypeInstruction decoded = decodeIType(code);
    
    // 验证往返
    ASSERT_EQ(decoded.opcode, Opcode::ADDI);
    ASSERT_EQ(decoded.rd, Register::R5);
    ASSERT_EQ(decoded.rs, Register::R10);
    ASSERT_EQ(decoded.immediate, -12345);
}

TEST(roundtrip_j_type) {
    AsmEncoder encoder;
    
    // 编码
    uint32_t code = encoder.encodeJType(Opcode::JAL, 123456);
    
    // 解码
    JTypeInstruction decoded = decodeJType(code);
    
    // 验证往返
    ASSERT_EQ(decoded.opcode, Opcode::JAL);
    ASSERT_EQ(decoded.target, 123456u);
}

// ============================================================================
// 快捷编码函数测试
// ============================================================================

TEST(quick_encode_functions) {
    // R-type
    uint32_t code1 = quickEncodeRType(Opcode::ADD, Register::R1, Register::R2, Register::R3);
    ASSERT_EQ(extractOpcode(code1), Opcode::ADD);
    
    // I-type
    uint32_t code2 = quickEncodeIType(Opcode::ADDI, Register::R1, Register::R0, 100);
    ASSERT_EQ(extractOpcode(code2), Opcode::ADDI);
    ASSERT_EQ(extractImmediate(code2), 100);
    
    // J-type
    uint32_t code3 = quickEncodeJType(Opcode::J, 500);
    ASSERT_EQ(extractOpcode(code3), Opcode::J);
    ASSERT_EQ(extractTarget(code3), 500u);
}

// ============================================================================
// 输出格式测试
// ============================================================================

TEST(to_hex_string) {
    // 测试十六进制输出
    ASSERT_EQ(AsmEncoder::toHexString(0x12345678, true), "0x12345678");
    ASSERT_EQ(AsmEncoder::toHexString(0x12345678, false), "12345678");
    ASSERT_EQ(AsmEncoder::toHexString(0x00000000, true), "0x00000000");
    ASSERT_EQ(AsmEncoder::toHexString(0xFFFFFFFF, true), "0xFFFFFFFF");
}

TEST(to_binary_string) {
    // 测试二进制输出
    std::string binary = AsmEncoder::toBinaryString(0x00000000);
    ASSERT_EQ(binary.length(), 36u);  // 32 bits + 4 spaces
    
    // 验证全 0
    std::string expected = "00000 00000 00000 00000 000000000000";
    ASSERT_EQ(binary, expected);
}

TEST(disassemble_fields) {
    AsmEncoder encoder;
    
    // 编码一条指令
    uint32_t code = encoder.encodeRType(Opcode::ADD, Register::R1, Register::R2, Register::R3);
    
    // 获取字段分解
    std::string fields = AsmEncoder::disassembleFields(code);
    
    // 验证包含关键信息
    ASSERT_TRUE(fields.find("add") != std::string::npos);
    ASSERT_TRUE(fields.find("R-type") != std::string::npos);
}

// ============================================================================
// 完整程序编码测试
// ============================================================================

TEST(encode_complete_program) {
    std::string program = R"(
.text
main:
    addi $r1, $r0, 10
    addi $r2, $r0, 20
    add $r3, $r1, $r2
    sw $r3, 0($r0)
    j end
end:
    output $r3
)";
    
    // 解析
    AsmParser parser(program);
    ParseResult parseResult = parser.parse();
    ASSERT_TRUE(parseResult.success);
    
    // 编码
    AsmEncoder encoder;
    EncodeResult encodeResult = encoder.encode(parseResult);
    
    // 验证编码成功
    ASSERT_TRUE(encodeResult.success);
    ASSERT_FALSE(encoder.hasErrors());
    
    // 验证指令数量
    ASSERT_EQ(encodeResult.instructions.size(), 6u);
    
    // 验证每条指令的操作码
    ASSERT_EQ(extractOpcode(encodeResult.instructions[0].machineCode), Opcode::ADDI);
    ASSERT_EQ(extractOpcode(encodeResult.instructions[1].machineCode), Opcode::ADDI);
    ASSERT_EQ(extractOpcode(encodeResult.instructions[2].machineCode), Opcode::ADD);
    ASSERT_EQ(extractOpcode(encodeResult.instructions[3].machineCode), Opcode::SW);
    ASSERT_EQ(extractOpcode(encodeResult.instructions[4].machineCode), Opcode::J);
    ASSERT_EQ(extractOpcode(encodeResult.instructions[5].machineCode), Opcode::OUTPUT);
}

TEST(encode_branch_instructions) {
    std::string program = R"(
.text
start:
    addi $r1, $r0, 5
loop:
    addi $r1, $r1, -1
    bgt $r1, $r0, loop
    j end
end:
    output $r1
)";
    
    // 解析
    AsmParser parser(program);
    ParseResult parseResult = parser.parse();
    ASSERT_TRUE(parseResult.success);
    
    // 编码
    AsmEncoder encoder;
    EncodeResult encodeResult = encoder.encode(parseResult);
    
    // 验证编码成功
    ASSERT_TRUE(encodeResult.success);
    
    // 验证分支指令的偏移量
    // bgt 在地址 2，目标 loop 在地址 1
    // 偏移 = 1 - (2 + 1) = -2
    uint32_t bgtCode = encodeResult.instructions[2].machineCode;
    ASSERT_EQ(extractOpcode(bgtCode), Opcode::BGT);
    ASSERT_EQ(extractImmediate(bgtCode), -2);
}

TEST(encode_with_label_resolution) {
    std::string program = R"(
.text
    j target
    addi $r1, $r0, 1
    addi $r2, $r0, 2
target:
    addi $r3, $r0, 3
)";
    
    // 解析
    AsmParser parser(program);
    ParseResult parseResult = parser.parse();
    ASSERT_TRUE(parseResult.success);
    
    // 编码
    AsmEncoder encoder;
    EncodeResult encodeResult = encoder.encode(parseResult);
    
    // 验证编码成功
    ASSERT_TRUE(encodeResult.success);
    
    // 验证 j 指令的目标地址
    // target 在地址 3
    uint32_t jCode = encodeResult.instructions[0].machineCode;
    ASSERT_EQ(extractOpcode(jCode), Opcode::J);
    ASSERT_EQ(extractTarget(jCode), 3u);
}

// ============================================================================
// 错误处理测试
// ============================================================================

TEST(encode_error_undefined_label) {
    std::string program = R"(
.text
    j undefined_label
)";
    
    // 解析
    AsmParser parser(program);
    ParseResult parseResult = parser.parse();
    
    // 解析可能成功（标签引用被记录），但编码应该失败
    if (parseResult.success) {
        AsmEncoder encoder;
        EncodeResult encodeResult = encoder.encode(parseResult);
        
        // 应该有未定义标签错误
        ASSERT_FALSE(encodeResult.success);
        ASSERT_TRUE(encoder.hasErrors());
    }
}

// ============================================================================
// 特殊指令测试
// ============================================================================

TEST(encode_io_instructions) {
    AsmEncoder encoder;
    
    // input $r1
    uint32_t inputCode = encoder.encodeIType(Opcode::INPUT, Register::R1, Register::R0, 0);
    ASSERT_EQ(extractOpcode(inputCode), Opcode::INPUT);
    ASSERT_EQ(extractRd(inputCode), Register::R1);
    
    // output $r2
    uint32_t outputCode = encoder.encodeIType(Opcode::OUTPUT, Register::R2, Register::R0, 0);
    ASSERT_EQ(extractOpcode(outputCode), Opcode::OUTPUT);
    ASSERT_EQ(extractRd(outputCode), Register::R2);
}

TEST(encode_memory_instructions) {
    AsmEncoder encoder;
    
    // lw $r1, 100($r2)
    uint32_t lwCode = encoder.encodeIType(Opcode::LW, Register::R1, Register::R2, 100);
    ASSERT_EQ(extractOpcode(lwCode), Opcode::LW);
    ASSERT_EQ(extractRd(lwCode), Register::R1);
    ASSERT_EQ(extractRs(lwCode), Register::R2);
    ASSERT_EQ(extractImmediate(lwCode), 100);
    
    // sw $r3, -50($r4)
    uint32_t swCode = encoder.encodeIType(Opcode::SW, Register::R3, Register::R4, -50);
    ASSERT_EQ(extractOpcode(swCode), Opcode::SW);
    ASSERT_EQ(extractRd(swCode), Register::R3);
    ASSERT_EQ(extractRs(swCode), Register::R4);
    ASSERT_EQ(extractImmediate(swCode), -50);
}

// ============================================================================
// 位域精确性测试
// ============================================================================

TEST(bit_field_positions) {
    AsmEncoder encoder;
    
    // 测试每个字段的精确位置
    // R-type: add $r31, $r15, $r7
    uint32_t code = encoder.encodeRType(Opcode::ADD, Register::R31, Register::R15, Register::R7);
    
    // opcode = 0 at bits [31:27]
    ASSERT_EQ((code >> 27) & 0x1F, 0u);
    
    // rd = 31 at bits [26:22]
    ASSERT_EQ((code >> 22) & 0x1F, 31u);
    
    // rs = 15 at bits [21:17]
    ASSERT_EQ((code >> 17) & 0x1F, 15u);
    
    // rt = 7 at bits [16:12]
    ASSERT_EQ((code >> 12) & 0x1F, 7u);
    
    // zeros at bits [11:0]
    ASSERT_EQ(code & 0xFFF, 0u);
}

TEST(immediate_sign_extension) {
    AsmEncoder encoder;
    
    // 测试立即数的符号扩展
    // 正数
    uint32_t code1 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, 100);
    ASSERT_EQ(extractImmediate(code1), 100);
    
    // 负数 -1 (17位: 0x1FFFF)
    uint32_t code2 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, -1);
    ASSERT_EQ(extractImmediate(code2), -1);
    
    // 边界值 -65536
    uint32_t code3 = encoder.encodeIType(Opcode::ADDI, Register::R1, Register::R0, -65536);
    ASSERT_EQ(extractImmediate(code3), -65536);
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "   AsmEncoder 单元测试" << std::endl;
    std::cout << "============================================" << std::endl;
    
    // R-type 编码测试
    std::cout << "\n--- R-type 编码测试 ---" << std::endl;
    RUN_TEST(encode_r_type_add);
    RUN_TEST(encode_r_type_sub);
    RUN_TEST(encode_r_type_and);
    RUN_TEST(encode_r_type_or);
    RUN_TEST(encode_r_type_sll);
    RUN_TEST(encode_r_type_srl);
    RUN_TEST(encode_r_type_all_registers);
    
    // I-type 编码测试
    std::cout << "\n--- I-type 编码测试 ---" << std::endl;
    RUN_TEST(encode_i_type_addi);
    RUN_TEST(encode_i_type_addi_negative);
    RUN_TEST(encode_i_type_lw);
    RUN_TEST(encode_i_type_sw);
    RUN_TEST(encode_i_type_beq);
    RUN_TEST(encode_i_type_bgt);
    RUN_TEST(encode_i_type_jr);
    RUN_TEST(encode_i_type_input);
    RUN_TEST(encode_i_type_output);
    RUN_TEST(encode_i_type_immediate_range);
    
    // J-type 编码测试
    std::cout << "\n--- J-type 编码测试 ---" << std::endl;
    RUN_TEST(encode_j_type_j);
    RUN_TEST(encode_j_type_jal);
    RUN_TEST(encode_j_type_target_range);
    
    // 往返测试
    std::cout << "\n--- 编码解码往返测试 ---" << std::endl;
    RUN_TEST(roundtrip_r_type);
    RUN_TEST(roundtrip_i_type);
    RUN_TEST(roundtrip_j_type);
    
    // 快捷函数测试
    std::cout << "\n--- 快捷编码函数测试 ---" << std::endl;
    RUN_TEST(quick_encode_functions);
    
    // 输出格式测试
    std::cout << "\n--- 输出格式测试 ---" << std::endl;
    RUN_TEST(to_hex_string);
    RUN_TEST(to_binary_string);
    RUN_TEST(disassemble_fields);
    
    // 完整程序测试
    std::cout << "\n--- 完整程序编码测试 ---" << std::endl;
    RUN_TEST(encode_complete_program);
    RUN_TEST(encode_branch_instructions);
    RUN_TEST(encode_with_label_resolution);
    
    // 错误处理测试
    std::cout << "\n--- 错误处理测试 ---" << std::endl;
    RUN_TEST(encode_error_undefined_label);
    
    // 特殊指令测试
    std::cout << "\n--- 特殊指令测试 ---" << std::endl;
    RUN_TEST(encode_io_instructions);
    RUN_TEST(encode_memory_instructions);
    
    // 位域精确性测试
    std::cout << "\n--- 位域精确性测试 ---" << std::endl;
    RUN_TEST(bit_field_positions);
    RUN_TEST(immediate_sign_extension);
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "   所有测试通过！" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
