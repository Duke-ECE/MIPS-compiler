/*
codegen_tests.cpp: CodeGen 单元测试
作用：测试从IR生成汇编代码的功能
*/

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>

// 引入要测试的模块
#include "compiler/ir.hpp"
#include "compiler/codegen.hpp"

// 简单的测试框架
class TestFramework {
private:
    int totalTests = 0;
    int passedTests = 0;
    std::string currentSuite;

public:
    void startSuite(const std::string &name) {
        currentSuite = name;
        std::cout << "\n=== " << name << " ===" << std::endl;
    }

    void test(const std::string &name, bool condition) {
        totalTests++;
        std::cout << "[" << currentSuite << "] " << name << ": ";
        if (condition) {
            std::cout << "PASS" << std::endl;
            passedTests++;
        } else {
            std::cout << "FAIL" << std::endl;
        }
    }

    void summary() {
        std::cout << "\n=== 测试总结 ===" << std::endl;
        std::cout << "总测试: " << totalTests << std::endl;
        std::cout << "通过: " << passedTests << std::endl;
        std::cout << "失败: " << (totalTests - passedTests) << std::endl;
        std::cout << "成功率: " << (passedTests * 100 / totalTests) << "%" << std::endl;
    }

    bool allPassed() const {
        return totalTests == passedTests;
    }
};

// 辅助函数：创建IR程序并生成汇编
std::vector<std::string> generateAssembly(const std::vector<IRInstruction> &instructions) {
    IRProgram program;
    for (const auto &inst : instructions) {
        program.instructions.push_back(inst);
    }
    
    CodeGen codegen;
    return codegen.generateAssembly(program);
}

// 辅助函数：检查汇编代码中是否包含特定指令
bool hasInstruction(const std::vector<std::string> &asm_code, const std::string &instruction) {
    for (const auto &line : asm_code) {
        if (line.find(instruction) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// 辅助函数：统计特定指令的数量
int countInstruction(const std::vector<std::string> &asm_code, const std::string &instruction) {
    int count = 0;
    for (const auto &line : asm_code) {
        if (line.find(instruction) != std::string::npos) {
            count++;
        }
    }
    return count;
}

// 辅助函数：查找指令在汇编中的位置
int findInstruction(const std::vector<std::string> &asm_code, const std::string &instruction) {
    for (size_t i = 0; i < asm_code.size(); ++i) {
        if (asm_code[i].find(instruction) != std::string::npos) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// 辅助函数：打印汇编代码（用于调试）
void printAssembly(const std::vector<std::string> &asm_code) {
    std::cout << "\n--- Assembly Code ---" << std::endl;
    for (size_t i = 0; i < asm_code.size(); ++i) {
        std::cout << i << ": " << asm_code[i] << std::endl;
    }
    std::cout << "--- End Assembly ---\n" << std::endl;
}

// 测试基础指令生成
void testBasicInstructions(TestFramework &tf) {
    tf.startSuite("基础指令生成测试");

    // 测试标签生成
    {
        auto asm_code = generateAssembly({{"label", "main", "", ""}});
        tf.test("标签生成", hasInstruction(asm_code, "main:"));
    }

    // 测试无条件跳转
    {
        auto asm_code = generateAssembly({{"jmp", "end", "", ""}});
        tf.test("无条件跳转生成", hasInstruction(asm_code, "j end"));
    }

    // 测试带返回值的return
    {
        auto asm_code = generateAssembly({{"ret", "t0", "", ""}});
        tf.test("带返回值的return", hasInstruction(asm_code, "jr $r"));
    }

    // 测试空return
    {
        auto asm_code = generateAssembly({{"ret", "", "", ""}});
        tf.test("空return", hasInstruction(asm_code, "jr $r31"));
    }
}

// 测试移动指令
void testMoveInstructions(TestFramework &tf) {
    tf.startSuite("移动指令测试");

    // 测试立即数移动
    {
        auto asm_code = generateAssembly({{"mov", "x", "42", ""}});
        tf.test("立即数移动", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, "42"));
    }

    // 测试寄存器移动
    {
        auto asm_code = generateAssembly({
            {"mov", "x", "5", ""},
            {"mov", "y", "x", ""}
        });
        tf.test("寄存器移动", countInstruction(asm_code, "addi") >= 2);
    }

    // 测试负数立即数
    {
        auto asm_code = generateAssembly({{"mov", "x", "-10", ""}});
        tf.test("负数立即数", hasInstruction(asm_code, "-10"));
    }
}

// 测试算术指令
void testArithmeticInstructions(TestFramework &tf) {
    tf.startSuite("算术指令测试");

    // 测试加法
    {
        auto asm_code = generateAssembly({{"add", "result", "x", "y"}});
        tf.test("加法指令", hasInstruction(asm_code, "add $r"));
    }

    // 测试减法
    {
        auto asm_code = generateAssembly({{"sub", "result", "x", "y"}});
        tf.test("减法指令", hasInstruction(asm_code, "sub $r"));
    }

    // 测试乘法（展开为循环）
    {
        auto asm_code = generateAssembly({{"mul", "result", "x", "y"}});
        tf.test("乘法包含循环标签", hasInstruction(asm_code, "LMD_LOOP_"));
        tf.test("乘法包含结束标签", hasInstruction(asm_code, "LMD_END_"));
        tf.test("乘法包含beq", hasInstruction(asm_code, "beq"));
        tf.test("乘法包含add", hasInstruction(asm_code, "add"));
    }

    // 测试除法（展开为循环）
    {
        auto asm_code = generateAssembly({{"div", "result", "x", "y"}});
        tf.test("除法包含循环标签", hasInstruction(asm_code, "LMD_LOOP_"));
        tf.test("除法包含bgt", hasInstruction(asm_code, "bgt"));
        tf.test("除法包含sub", hasInstruction(asm_code, "sub"));
    }
}

// 测试一元运算
void testUnaryOperations(TestFramework &tf) {
    tf.startSuite("一元运算测试");

    // 测试负号
    {
        auto asm_code = generateAssembly({{"neg", "result", "x", ""}});
        tf.test("负号运算", hasInstruction(asm_code, "sub") && hasInstruction(asm_code, "$r0"));
    }

    // 测试逻辑非
    {
        auto asm_code = generateAssembly({{"not", "result", "x", ""}});
        tf.test("逻辑非包含条件分支", hasInstruction(asm_code, "beq"));
        tf.test("逻辑非包含标签", hasInstruction(asm_code, "LNOT_"));
        tf.test("逻辑非设置0", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, ", 0"));
        tf.test("逻辑非设置1", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, ", 1"));
    }
}

// 测试比较操作
void testComparisonOperations(TestFramework &tf) {
    tf.startSuite("比较操作测试");

    // 测试小于
    {
        auto asm_code = generateAssembly({{"lt", "result", "x", "y"}});
        tf.test("小于比较包含bgt", hasInstruction(asm_code, "bgt"));
        tf.test("小于比较包含标签", hasInstruction(asm_code, "LCMP_"));
    }

    // 测试大于
    {
        auto asm_code = generateAssembly({{"gt", "result", "x", "y"}});
        tf.test("大于比较包含bgt", hasInstruction(asm_code, "bgt"));
    }

    // 测试等于
    {
        auto asm_code = generateAssembly({{"eq", "result", "x", "y"}});
        tf.test("等于比较包含beq", hasInstruction(asm_code, "beq"));
    }

    // 测试不等于
    {
        auto asm_code = generateAssembly({{"ne", "result", "x", "y"}});
        tf.test("不等于比较包含beq", hasInstruction(asm_code, "beq"));
    }

    // 测试小于等于
    {
        auto asm_code = generateAssembly({{"le", "result", "x", "y"}});
        tf.test("小于等于包含bgt", hasInstruction(asm_code, "bgt"));
        tf.test("小于等于包含标签", hasInstruction(asm_code, "LCMP_"));
    }

    // 测试大于等于
    {
        auto asm_code = generateAssembly({{"ge", "result", "x", "y"}});
        tf.test("大于等于包含bgt", hasInstruction(asm_code, "bgt"));
    }

    // 验证所有比较操作都设置结果为0或1
    {
        auto asm_code = generateAssembly({{"lt", "result", "x", "y"}});
        tf.test("比较操作设置0", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, ", 0"));
        tf.test("比较操作设置1", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, ", 1"));
    }
}

// 测试函数调用
void testFunctionCalls(TestFramework &tf) {
    tf.startSuite("函数调用测试");

    // 测试函数调用
    {
        auto asm_code = generateAssembly({{"call", "result", "func_name", "2"}});
        tf.test("函数调用包含jal", hasInstruction(asm_code, "jal func_name"));
        tf.test("函数调用保存返回值", hasInstruction(asm_code, "$r31"));
    }
}

// 测试I/O操作
void testIOOperations(TestFramework &tf) {
    tf.startSuite("I/O操作测试");

    // 测试输入
    {
        auto asm_code = generateAssembly({{"input", "x", "", ""}});
        tf.test("输入操作", hasInstruction(asm_code, "input $r"));
    }

    // 测试输出
    {
        auto asm_code = generateAssembly({{"output", "x", "", ""}});
        tf.test("输出操作", hasInstruction(asm_code, "output $r"));
    }
}

// 测试寄存器分配
void testRegisterAllocation(TestFramework &tf) {
    tf.startSuite("寄存器分配测试");

    // 测试多个变量的寄存器分配
    {
        auto asm_code = generateAssembly({
            {"mov", "a", "1", ""},
            {"mov", "b", "2", ""},
            {"mov", "c", "3", ""},
            {"add", "d", "a", "b"}
        });
        
        tf.test("多变量寄存器分配", hasInstruction(asm_code, "$r1") && 
                                    hasInstruction(asm_code, "$r2") && 
                                    hasInstruction(asm_code, "$r3"));
    }

    // 测试变量重用
    {
        auto asm_code = generateAssembly({
            {"mov", "x", "5", ""},
            {"add", "y", "x", "x"}  // x被重用
        });
        tf.test("变量重用相同寄存器", countInstruction(asm_code, "$r1") >= 2);
    }
}

// 测试复杂程序
void testComplexPrograms(TestFramework &tf) {
    tf.startSuite("复杂程序测试");

    // 测试简单函数
    {
        auto asm_code = generateAssembly({
            {"label", "main", "", ""},
            {"mov", "t0", "42", ""},
            {"ret", "t0", "", ""}
        });
        tf.test("简单函数包含标签", hasInstruction(asm_code, "main:"));
        tf.test("简单函数包含移动", hasInstruction(asm_code, "addi"));
        tf.test("简单函数包含返回", hasInstruction(asm_code, "jr"));
    }

    // 测试算术表达式
    {
        auto asm_code = generateAssembly({
            {"label", "main", "", ""},
            {"mov", "t0", "1", ""},
            {"mov", "t1", "2", ""},
            {"add", "t2", "t0", "t1"},
            {"ret", "t2", "", ""}
        });
        tf.test("表达式包含加法", hasInstruction(asm_code, "add"));
        tf.test("表达式包含多个移动", countInstruction(asm_code, "addi") >= 2);
    }

    // 测试条件分支模拟
    {
        auto asm_code = generateAssembly({
            {"label", "main", "", ""},
            {"mov", "t0", "1", ""},
            {"mov", "t1", "2", ""},
            {"lt", "t2", "t0", "t1"},
            {"ret", "t2", "", ""}
        });
        tf.test("条件分支包含比较", hasInstruction(asm_code, "bgt"));
        tf.test("条件分支包含条件标签", hasInstruction(asm_code, "LCMP_"));
    }

    // 测试循环结构（乘法展开）
    {
        auto asm_code = generateAssembly({
            {"label", "main", "", ""},
            {"mov", "a", "3", ""},
            {"mov", "b", "4", ""},
            {"mul", "result", "a", "b"},
            {"ret", "result", "", ""}
        });
        tf.test("循环结构包含循环标签", hasInstruction(asm_code, "LMD_LOOP_"));
        tf.test("循环结构包含条件跳转", hasInstruction(asm_code, "beq"));
        tf.test("循环结构包含循环体", hasInstruction(asm_code, "add"));
    }
}

// 测试边界情况
void testEdgeCases(TestFramework &tf) {
    tf.startSuite("边界情况测试");

    // 测试空程序
    {
        auto asm_code = generateAssembly({});
        tf.test("空程序生成空汇编", asm_code.empty());
    }

    // 测试只有标签
    {
        auto asm_code = generateAssembly({{"label", "start", "", ""}});
        tf.test("只有标签", asm_code.size() == 1 && hasInstruction(asm_code, "start:"));
    }

    // 测试大数字
    {
        auto asm_code = generateAssembly({{"mov", "x", "123456", ""}});
        tf.test("大数字处理", hasInstruction(asm_code, "123456"));
    }

    // 测试零值
    {
        auto asm_code = generateAssembly({{"mov", "x", "0", ""}});
        tf.test("零值处理", hasInstruction(asm_code, "addi") && hasInstruction(asm_code, ", 0"));
    }

    // 测试多个连续的同类型操作
    {
        auto asm_code = generateAssembly({
            {"add", "r1", "a", "b"},
            {"add", "r2", "c", "d"},
            {"add", "r3", "e", "f"}
        });
        tf.test("多个加法操作", countInstruction(asm_code, "add") == 3);
    }
}

// 测试指令顺序
void testInstructionOrder(TestFramework &tf) {
    tf.startSuite("指令顺序测试");

    // 测试标签在跳转前
    {
        auto asm_code = generateAssembly({
            {"label", "start", "", ""},
            {"mov", "x", "1", ""},
            {"jmp", "end", "", ""},
            {"label", "end", "", ""}
        });
        
        int start_pos = findInstruction(asm_code, "start:");
        int jmp_pos = findInstruction(asm_code, "j end");
        int end_pos = findInstruction(asm_code, "end:");
        
        tf.test("标签顺序正确", start_pos < jmp_pos && jmp_pos < end_pos);
    }

    // 测试比较操作的分支结构
    {
        auto asm_code = generateAssembly({{"lt", "result", "x", "y"}});
        
        int bgt_pos = findInstruction(asm_code, "bgt");
        int true_label_pos = -1;
        int end_label_pos = -1;
        
        // 查找标签位置（包含冒号）
        for (size_t i = 0; i < asm_code.size(); ++i) {
            if (asm_code[i].find("LCMP_T_") != std::string::npos && asm_code[i].find(":") != std::string::npos) {
                true_label_pos = static_cast<int>(i);
            }
            if (asm_code[i].find("LCMP_E_") != std::string::npos && asm_code[i].find(":") != std::string::npos) {
                end_label_pos = static_cast<int>(i);
            }
        }
        
        tf.test("比较分支结构正确", bgt_pos >= 0 && true_label_pos >= 0 && end_label_pos >= 0 && 
                                   bgt_pos < true_label_pos && true_label_pos < end_label_pos);
    }
}

// 测试错误处理
void testErrorHandling(TestFramework &tf) {
    tf.startSuite("错误处理测试");

    // 测试未知指令
    {
        bool caught_error = false;
        try {
            auto asm_code = generateAssembly({{"unknown_op", "dst", "src1", "src2"}});
        } catch (const std::runtime_error &e) {
            caught_error = true;
        } catch (...) {
            caught_error = true;
        }
        tf.test("未知指令抛出异常", caught_error);
    }
}

// 测试特定汇编模式
void testAssemblyPatterns(TestFramework &tf) {
    tf.startSuite("汇编模式测试");

    // 测试寄存器命名模式
    {
        auto asm_code = generateAssembly({{"mov", "x", "5", ""}});
        bool has_reg_pattern = false;
        for (const auto &line : asm_code) {
            if (line.find("$r") != std::string::npos) {
                has_reg_pattern = true;
                break;
            }
        }
        tf.test("寄存器命名模式正确", has_reg_pattern);
    }

    // 测试标签命名模式
    {
        auto asm_code = generateAssembly({{"label", "function_name", "", ""}});
        tf.test("标签格式正确", hasInstruction(asm_code, "function_name:"));
    }

    // 测试立即数格式
    {
        auto asm_code = generateAssembly({{"mov", "x", "42", ""}});
        bool has_immediate = false;
        for (const auto &line : asm_code) {
            if (line.find(", 42") != std::string::npos) {
                has_immediate = true;
                break;
            }
        }
        tf.test("立即数格式正确", has_immediate);
    }

    // 测试跳转目标格式
    {
        auto asm_code = generateAssembly({{"jmp", "target", "", ""}});
        tf.test("跳转目标格式正确", hasInstruction(asm_code, "j target"));
    }
}

// 主测试函数
int main() {
    TestFramework tf;
    
    std::cout << "MIPS 编译器 CodeGen 单元测试" << std::endl;
    std::cout << "==============================" << std::endl;

    // 运行所有测试
    testBasicInstructions(tf);
    testMoveInstructions(tf);
    testArithmeticInstructions(tf);
    testUnaryOperations(tf);
    testComparisonOperations(tf);
    testFunctionCalls(tf);
    testIOOperations(tf);
    testRegisterAllocation(tf);
    testComplexPrograms(tf);
    testEdgeCases(tf);
    testInstructionOrder(tf);
    testErrorHandling(tf);
    testAssemblyPatterns(tf);

    // 显示测试结果
    tf.summary();

    return tf.allPassed() ? 0 : 1;
}
