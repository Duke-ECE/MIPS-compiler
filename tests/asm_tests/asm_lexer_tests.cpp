/**
 * @file asm_lexer_tests.cpp
 * @brief AsmLexer 单元测试
 * 
 * 测试覆盖：
 * - 指令解析
 * - 寄存器解析
 * - 数字解析（十进制、十六进制、二进制）
 * - 标签解析
 * - 注释处理
 * - 汇编指令解析
 * - 完整汇编程序解析
 */

#include "assembler/asm_lexer.hpp"
#include "assembler/asm_token.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace assembler;

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

// 辅助函数：检查 Token
void assertToken(const AsmToken& token, AsmTokenType expectedType, 
                 const std::string& expectedText, int expectedLine = -1, 
                 int expectedColumn = -1) {
    if (token.type != expectedType) {
        std::cerr << "\nToken 类型不匹配. 期望: " << asmTokenTypeToString(expectedType)
                  << ", 实际: " << asmTokenTypeToString(token.type) << std::endl;
        std::cerr << "Token 文本: \"" << token.text << "\"" << std::endl;
        assert(false);
    }
    if (token.text != expectedText) {
        std::cerr << "\nToken 文本不匹配. 期望: \"" << expectedText 
                  << "\", 实际: \"" << token.text << "\"" << std::endl;
        assert(false);
    }
    if (expectedLine != -1 && token.line != expectedLine) {
        std::cerr << "\nToken 行号不匹配. 期望: " << expectedLine 
                  << ", 实际: " << token.line << std::endl;
        assert(false);
    }
    if (expectedColumn != -1 && token.column != expectedColumn) {
        std::cerr << "\nToken 列号不匹配. 期望: " << expectedColumn 
                  << ", 实际: " << token.column << std::endl;
        assert(false);
    }
}

// ============================================================================
// 指令测试
// ============================================================================

TEST(instruction_recognition) {
    // 测试所有 ISA 指令
    std::vector<std::string> instructions = {
        "add", "sub", "and", "or", "sll", "srl",
        "addi", "lw", "sw", "beq", "bgt", "jr",
        "j", "jal", "input", "output"
    };
    
    for (const auto& instr : instructions) {
        AsmLexer lexer(instr);
        AsmToken token = lexer.nextToken();
        assertToken(token, AsmTokenType::INSTRUCTION, instr);
    }
}

TEST(instruction_case_insensitive) {
    // 测试大小写不敏感（ISA 模块处理）
    AsmLexer lexer1("ADD");
    AsmToken t1 = lexer1.nextToken();
    assertToken(t1, AsmTokenType::INSTRUCTION, "ADD");
    
    AsmLexer lexer2("AdD");
    AsmToken t2 = lexer2.nextToken();
    assertToken(t2, AsmTokenType::INSTRUCTION, "AdD");
}

// ============================================================================
// 寄存器测试
// ============================================================================

TEST(register_numeric) {
    // 测试数字格式寄存器 $r0 - $r31
    for (int i = 0; i < 32; i++) {
        std::string regStr = "$r" + std::to_string(i);
        AsmLexer lexer(regStr);
        AsmToken token = lexer.nextToken();
        assertToken(token, AsmTokenType::REGISTER, regStr);
    }
}

TEST(register_aliases) {
    // 测试寄存器别名
    std::vector<std::string> aliases = {
        "$zero", "$ra", "$sp", "$fp", "$gp",
        "$t0", "$t1", "$s0", "$s1", "$a0", "$v0"
    };
    
    for (const auto& alias : aliases) {
        AsmLexer lexer(alias);
        AsmToken token = lexer.nextToken();
        assertToken(token, AsmTokenType::REGISTER, alias);
    }
}

TEST(register_invalid) {
    // 测试无效寄存器
    AsmLexer lexer("$r32");  // 超出范围
    AsmToken token = lexer.nextToken();
    ASSERT_EQ(token.type, AsmTokenType::INVALID);
}

// ============================================================================
// 数字测试
// ============================================================================

TEST(number_decimal) {
    // 测试十进制数字
    AsmLexer lexer("0 123 456789");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 4u); // 3 numbers + EOF
    assertToken(tokens[0], AsmTokenType::NUMBER, "0");
    ASSERT_EQ(tokens[0].numericValue, 0);
    
    assertToken(tokens[1], AsmTokenType::NUMBER, "123");
    ASSERT_EQ(tokens[1].numericValue, 123);
    
    assertToken(tokens[2], AsmTokenType::NUMBER, "456789");
    ASSERT_EQ(tokens[2].numericValue, 456789);
}

TEST(number_negative) {
    // 测试负数
    AsmLexer lexer("-123 -1 -65536");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 4u); // 3 numbers + EOF
    assertToken(tokens[0], AsmTokenType::NUMBER, "-123");
    ASSERT_EQ(tokens[0].numericValue, -123);
    
    assertToken(tokens[1], AsmTokenType::NUMBER, "-1");
    ASSERT_EQ(tokens[1].numericValue, -1);
    
    assertToken(tokens[2], AsmTokenType::NUMBER, "-65536");
    ASSERT_EQ(tokens[2].numericValue, -65536);
}

TEST(number_hexadecimal) {
    // 测试十六进制数字
    AsmLexer lexer("0x0 0x1A 0xFF 0xABCD");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 5u); // 4 numbers + EOF
    assertToken(tokens[0], AsmTokenType::NUMBER, "0x0");
    ASSERT_EQ(tokens[0].numericValue, 0);
    
    assertToken(tokens[1], AsmTokenType::NUMBER, "0x1A");
    ASSERT_EQ(tokens[1].numericValue, 0x1A);
    
    assertToken(tokens[2], AsmTokenType::NUMBER, "0xFF");
    ASSERT_EQ(tokens[2].numericValue, 0xFF);
    
    assertToken(tokens[3], AsmTokenType::NUMBER, "0xABCD");
    ASSERT_EQ(tokens[3].numericValue, 0xABCD);
}

TEST(number_binary) {
    // 测试二进制数字
    AsmLexer lexer("0b0 0b1 0b1010 0b11111111");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 5u); // 4 numbers + EOF
    assertToken(tokens[0], AsmTokenType::NUMBER, "0b0");
    ASSERT_EQ(tokens[0].numericValue, 0);
    
    assertToken(tokens[1], AsmTokenType::NUMBER, "0b1");
    ASSERT_EQ(tokens[1].numericValue, 1);
    
    assertToken(tokens[2], AsmTokenType::NUMBER, "0b1010");
    ASSERT_EQ(tokens[2].numericValue, 0b1010);
    
    assertToken(tokens[3], AsmTokenType::NUMBER, "0b11111111");
    ASSERT_EQ(tokens[3].numericValue, 0b11111111);
}

// ============================================================================
// 标签测试
// ============================================================================

TEST(label_definition) {
    // 测试标签定义
    AsmLexer lexer("main: loop: _end_func:");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 4u); // 3 labels + EOF
    assertToken(tokens[0], AsmTokenType::LABEL, "main");
    assertToken(tokens[1], AsmTokenType::LABEL, "loop");
    assertToken(tokens[2], AsmTokenType::LABEL, "_end_func");
}

TEST(label_reference) {
    // 测试标签引用（没有冒号）
    AsmLexer lexer("j loop");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 3u); // instruction + identifier + EOF
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "j");
    assertToken(tokens[1], AsmTokenType::IDENTIFIER, "loop");
}

// ============================================================================
// 注释测试
// ============================================================================

TEST(comment_hash) {
    // 测试 # 注释
    AsmLexer lexer("add $r1, $r2, $r3 # 这是注释");
    std::vector<AsmToken> tokens = lexer.tokenize(false, true);
    
    // instruction + 3 registers + 2 commas + comment + EOF
    bool hasComment = false;
    for (const auto& t : tokens) {
        if (t.type == AsmTokenType::COMMENT) {
            hasComment = true;
            ASSERT_TRUE(t.text.find("这是注释") != std::string::npos);
        }
    }
    ASSERT_TRUE(hasComment);
}

TEST(comment_semicolon) {
    // 测试 ; 注释
    AsmLexer lexer("add $r1, $r2, $r3 ; comment");
    std::vector<AsmToken> tokens = lexer.tokenize(false, true);
    
    bool hasComment = false;
    for (const auto& t : tokens) {
        if (t.type == AsmTokenType::COMMENT) {
            hasComment = true;
        }
    }
    ASSERT_TRUE(hasComment);
}

TEST(comment_excluded) {
    // 测试注释默认被排除
    AsmLexer lexer("add $r1, $r2, $r3 # comment");
    std::vector<AsmToken> tokens = lexer.tokenize(false, false);
    
    for (const auto& t : tokens) {
        ASSERT_TRUE(t.type != AsmTokenType::COMMENT);
    }
}

// ============================================================================
// 汇编指令测试
// ============================================================================

TEST(directive_parsing) {
    // 测试汇编指令
    AsmLexer lexer(".data .text .word .byte");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 5u); // 4 directives + EOF
    assertToken(tokens[0], AsmTokenType::DIRECTIVE, ".data");
    assertToken(tokens[1], AsmTokenType::DIRECTIVE, ".text");
    assertToken(tokens[2], AsmTokenType::DIRECTIVE, ".word");
    assertToken(tokens[3], AsmTokenType::DIRECTIVE, ".byte");
}

// ============================================================================
// 分隔符测试
// ============================================================================

TEST(delimiters) {
    // 测试分隔符
    AsmLexer lexer(", ( ) :");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 5u); // 4 delimiters + EOF
    assertToken(tokens[0], AsmTokenType::COMMA, ",");
    assertToken(tokens[1], AsmTokenType::LPAREN, "(");
    assertToken(tokens[2], AsmTokenType::RPAREN, ")");
    assertToken(tokens[3], AsmTokenType::COLON, ":");
}

TEST(newline_handling) {
    // 测试换行处理
    AsmLexer lexer("add $r1, $r2, $r3\nsub $r4, $r5, $r6");
    std::vector<AsmToken> tokens = lexer.tokenize(true);
    
    bool hasNewline = false;
    for (const auto& t : tokens) {
        if (t.type == AsmTokenType::NEWLINE) {
            hasNewline = true;
            break;
        }
    }
    ASSERT_TRUE(hasNewline);
}

// ============================================================================
// 字符串测试
// ============================================================================

TEST(string_literal) {
    // 测试字符串字面量
    AsmLexer lexer("\"hello world\"");
    AsmToken token = lexer.nextToken();
    assertToken(token, AsmTokenType::STRING, "\"hello world\"");
}

TEST(string_with_escape) {
    // 测试带转义的字符串
    AsmLexer lexer("\"hello\\nworld\"");
    AsmToken token = lexer.nextToken();
    assertToken(token, AsmTokenType::STRING, "\"hello\\nworld\"");
}

// ============================================================================
// R-type 指令完整测试
// ============================================================================

TEST(r_type_instruction) {
    // 测试 R-type 指令: add $rd, $rs, $rt
    AsmLexer lexer("add $r1, $r2, $r3");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 7u); // add + 3 regs + 2 commas + EOF
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "add");
    assertToken(tokens[1], AsmTokenType::REGISTER, "$r1");
    assertToken(tokens[2], AsmTokenType::COMMA, ",");
    assertToken(tokens[3], AsmTokenType::REGISTER, "$r2");
    assertToken(tokens[4], AsmTokenType::COMMA, ",");
    assertToken(tokens[5], AsmTokenType::REGISTER, "$r3");
    assertToken(tokens[6], AsmTokenType::END_OF_FILE, "");
}

// ============================================================================
// I-type 指令完整测试
// ============================================================================

TEST(i_type_instruction_addi) {
    // 测试 I-type 指令: addi $rd, $rs, imm
    AsmLexer lexer("addi $r1, $r2, 100");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 7u); // addi + 2 regs + imm + 2 commas + EOF
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "addi");
    assertToken(tokens[1], AsmTokenType::REGISTER, "$r1");
    assertToken(tokens[2], AsmTokenType::COMMA, ",");
    assertToken(tokens[3], AsmTokenType::REGISTER, "$r2");
    assertToken(tokens[4], AsmTokenType::COMMA, ",");
    assertToken(tokens[5], AsmTokenType::NUMBER, "100");
    ASSERT_EQ(tokens[5].numericValue, 100);
}

TEST(i_type_instruction_lw) {
    // 测试 I-type 指令: lw $rd, offset($rs)
    AsmLexer lexer("lw $r4, 100($r5)");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 8u); // lw + 2 regs + number + comma + parens + EOF
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "lw");
    assertToken(tokens[1], AsmTokenType::REGISTER, "$r4");
    assertToken(tokens[2], AsmTokenType::COMMA, ",");
    assertToken(tokens[3], AsmTokenType::NUMBER, "100");
    assertToken(tokens[4], AsmTokenType::LPAREN, "(");
    assertToken(tokens[5], AsmTokenType::REGISTER, "$r5");
    assertToken(tokens[6], AsmTokenType::RPAREN, ")");
}

TEST(i_type_instruction_beq) {
    // 测试 I-type 分支指令: beq $rd, $rs, label
    AsmLexer lexer("beq $r1, $r2, loop");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 7u);
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "beq");
    assertToken(tokens[1], AsmTokenType::REGISTER, "$r1");
    assertToken(tokens[2], AsmTokenType::COMMA, ",");
    assertToken(tokens[3], AsmTokenType::REGISTER, "$r2");
    assertToken(tokens[4], AsmTokenType::COMMA, ",");
    assertToken(tokens[5], AsmTokenType::IDENTIFIER, "loop");
}

// ============================================================================
// J-type 指令完整测试
// ============================================================================

TEST(j_type_instruction) {
    // 测试 J-type 指令: j target
    AsmLexer lexer("j end");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 3u); // j + label + EOF
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "j");
    assertToken(tokens[1], AsmTokenType::IDENTIFIER, "end");
}

TEST(jal_instruction) {
    // 测试 JAL 指令: jal func
    AsmLexer lexer("jal my_function");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    ASSERT_EQ(tokens.size(), 3u);
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "jal");
    assertToken(tokens[1], AsmTokenType::IDENTIFIER, "my_function");
}

// ============================================================================
// I/O 指令测试
// ============================================================================

TEST(io_instructions) {
    // 测试 I/O 指令
    AsmLexer lexer("input $r1\noutput $r2");
    std::vector<AsmToken> tokens = lexer.tokenize(false);
    
    assertToken(tokens[0], AsmTokenType::INSTRUCTION, "input");
    assertToken(tokens[1], AsmTokenType::REGISTER, "$r1");
    assertToken(tokens[2], AsmTokenType::INSTRUCTION, "output");
    assertToken(tokens[3], AsmTokenType::REGISTER, "$r2");
}

// ============================================================================
// 完整程序测试
// ============================================================================

TEST(complete_program) {
    // 测试完整的汇编程序
    std::string program = R"(
# 简单的加法程序
.text
main:
    addi $r1, $r0, 10    # r1 = 10
    addi $r2, $r0, 20    # r2 = 20
    add $r3, $r1, $r2    # r3 = r1 + r2
    sw $r3, 0($r0)       # 保存结果
    j end
end:
    output $r3           # 输出结果
)";
    
    AsmLexer lexer(program);
    std::vector<AsmToken> tokens = lexer.tokenize(false, false);
    
    // 验证程序能正确解析
    bool hasDirective = false;
    bool hasLabel = false;
    bool hasInstruction = false;
    bool hasRegister = false;
    bool hasNumber = false;
    
    for (const auto& t : tokens) {
        switch (t.type) {
            case AsmTokenType::DIRECTIVE:   hasDirective = true; break;
            case AsmTokenType::LABEL:       hasLabel = true; break;
            case AsmTokenType::INSTRUCTION: hasInstruction = true; break;
            case AsmTokenType::REGISTER:    hasRegister = true; break;
            case AsmTokenType::NUMBER:      hasNumber = true; break;
            default: break;
        }
    }
    
    ASSERT_TRUE(hasDirective);
    ASSERT_TRUE(hasLabel);
    ASSERT_TRUE(hasInstruction);
    ASSERT_TRUE(hasRegister);
    ASSERT_TRUE(hasNumber);
}

// ============================================================================
// 行列号测试
// ============================================================================

TEST(line_column_tracking) {
    // 测试行列号追踪
    std::string code = "add $r1, $r2, $r3\nsub $r4, $r5, $r6";
    AsmLexer lexer(code);
    
    // 第一行
    AsmToken t1 = lexer.nextToken(); // add
    ASSERT_EQ(t1.line, 1);
    ASSERT_EQ(t1.column, 1);
    
    // 跳过第一行的其他 token
    while (lexer.nextToken().type != AsmTokenType::NEWLINE);
    
    // 第二行
    AsmToken t2 = lexer.nextToken(); // sub
    ASSERT_EQ(t2.line, 2);
    ASSERT_EQ(t2.column, 1);
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST(empty_input) {
    // 测试空输入
    AsmLexer lexer("");
    AsmToken token = lexer.nextToken();
    ASSERT_EQ(token.type, AsmTokenType::END_OF_FILE);
}

TEST(whitespace_only) {
    // 测试只有空白的输入
    AsmLexer lexer("   \t\t   ");
    AsmToken token = lexer.nextToken();
    ASSERT_EQ(token.type, AsmTokenType::END_OF_FILE);
}

TEST(only_newlines) {
    // 测试只有换行的输入
    AsmLexer lexer("\n\n\n");
    std::vector<AsmToken> tokens = lexer.tokenize(true);
    
    int newlineCount = 0;
    for (const auto& t : tokens) {
        if (t.type == AsmTokenType::NEWLINE) newlineCount++;
    }
    ASSERT_EQ(newlineCount, 3);
}

TEST(peek_token) {
    // 测试 peekToken 功能
    AsmLexer lexer("add $r1, $r2, $r3");
    
    AsmToken peeked = lexer.peekToken();
    assertToken(peeked, AsmTokenType::INSTRUCTION, "add");
    
    // 再次 peek 应该返回相同的 token
    AsmToken peeked2 = lexer.peekToken();
    assertToken(peeked2, AsmTokenType::INSTRUCTION, "add");
    
    // nextToken 应该消耗这个 token
    AsmToken consumed = lexer.nextToken();
    assertToken(consumed, AsmTokenType::INSTRUCTION, "add");
    
    // 下一个应该是寄存器
    AsmToken next = lexer.nextToken();
    assertToken(next, AsmTokenType::REGISTER, "$r1");
}

TEST(reset_functionality) {
    // 测试 reset 功能
    AsmLexer lexer("add sub");
    
    lexer.nextToken(); // add
    lexer.nextToken(); // sub
    
    lexer.reset();
    
    AsmToken token = lexer.nextToken();
    assertToken(token, AsmTokenType::INSTRUCTION, "add");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "   AsmLexer 单元测试" << std::endl;
    std::cout << "============================================" << std::endl;
    
    // 指令测试
    RUN_TEST(instruction_recognition);
    RUN_TEST(instruction_case_insensitive);
    
    // 寄存器测试
    RUN_TEST(register_numeric);
    RUN_TEST(register_aliases);
    RUN_TEST(register_invalid);
    
    // 数字测试
    RUN_TEST(number_decimal);
    RUN_TEST(number_negative);
    RUN_TEST(number_hexadecimal);
    RUN_TEST(number_binary);
    
    // 标签测试
    RUN_TEST(label_definition);
    RUN_TEST(label_reference);
    
    // 注释测试
    RUN_TEST(comment_hash);
    RUN_TEST(comment_semicolon);
    RUN_TEST(comment_excluded);
    
    // 汇编指令测试
    RUN_TEST(directive_parsing);
    
    // 分隔符测试
    RUN_TEST(delimiters);
    RUN_TEST(newline_handling);
    
    // 字符串测试
    RUN_TEST(string_literal);
    RUN_TEST(string_with_escape);
    
    // R-type 指令测试
    RUN_TEST(r_type_instruction);
    
    // I-type 指令测试
    RUN_TEST(i_type_instruction_addi);
    RUN_TEST(i_type_instruction_lw);
    RUN_TEST(i_type_instruction_beq);
    
    // J-type 指令测试
    RUN_TEST(j_type_instruction);
    RUN_TEST(jal_instruction);
    
    // I/O 指令测试
    RUN_TEST(io_instructions);
    
    // 完整程序测试
    RUN_TEST(complete_program);
    
    // 行列号测试
    RUN_TEST(line_column_tracking);
    
    // 边界情况测试
    RUN_TEST(empty_input);
    RUN_TEST(whitespace_only);
    RUN_TEST(only_newlines);
    RUN_TEST(peek_token);
    RUN_TEST(reset_functionality);
    
    std::cout << "============================================" << std::endl;
    std::cout << "   所有测试通过！" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
