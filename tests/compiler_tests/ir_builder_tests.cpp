/*
ir_builder_tests.cpp: IRBuilder 单元测试
作用：测试从AST生成中间代码（IR）的功能
*/

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>

// 引入要测试的模块
#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include "compiler/ast.hpp"
#include "compiler/ir.hpp"
#include "compiler/ir_builder.hpp"

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

// 辅助函数：解析代码并生成IR
IRProgram generateIR(const std::string &code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parseProgram();
    
    IRBuilder builder;
    return builder.generateIR(ast.get());
}

// 辅助函数：检查IR中是否包含特定指令
bool hasInstruction(const IRProgram &ir, const std::string &op, 
                   const std::string &dst = "", const std::string &src1 = "", 
                   const std::string &src2 = "") {
    for (const auto &inst : ir.instructions) {
        if (inst.op == op) {
            bool match = true;
            if (!dst.empty() && inst.dst != dst) match = false;
            if (!src1.empty() && inst.src1 != src1) match = false;
            if (!src2.empty() && inst.src2 != src2) match = false;
            if (match) return true;
        }
    }
    return false;
}

// 辅助函数：统计特定操作的指令数量
int countInstructions(const IRProgram &ir, const std::string &op) {
    int count = 0;
    for (const auto &inst : ir.instructions) {
        if (inst.op == op) count++;
    }
    return count;
}

// 辅助函数：打印IR指令（用于调试）
void printIR(const IRProgram &ir) {
    std::cout << "\n--- IR Instructions ---" << std::endl;
    for (const auto &inst : ir.instructions) {
        std::cout << inst.op;
        if (!inst.dst.empty()) std::cout << " " << inst.dst;
        if (!inst.src1.empty()) std::cout << " " << inst.src1;
        if (!inst.src2.empty()) std::cout << " " << inst.src2;
        std::cout << std::endl;
    }
    std::cout << "--- End IR ---\n" << std::endl;
}

// 辅助函数：查找指令在IR中的位置
int findInstruction(const IRProgram &ir, const std::string &op, 
                   const std::string &dst = "", const std::string &src1 = "", 
                   const std::string &src2 = "") {
    for (size_t i = 0; i < ir.instructions.size(); ++i) {
        const auto &inst = ir.instructions[i];
        if (inst.op == op) {
            bool match = true;
            if (!dst.empty() && inst.dst != dst) match = false;
            if (!src1.empty() && inst.src1 != src1) match = false;
            if (!src2.empty() && inst.src2 != src2) match = false;
            if (match) return static_cast<int>(i);
        }
    }
    return -1;
}

// 测试基础IR生成
void testBasicIRGeneration(TestFramework &tf) {
    tf.startSuite("基础IR生成测试");

    // 测试简单函数
    {
        auto ir = generateIR("int main() { return 42; }");
        tf.test("简单函数生成", ir.instructions.size() > 0);
        tf.test("包含函数标签", hasInstruction(ir, "label", "main"));
        tf.test("包含返回指令", hasInstruction(ir, "ret"));
        tf.test("包含数字移动指令", hasInstruction(ir, "mov"));
    }

    // 测试空函数
    {
        auto ir = generateIR("int main() { }");
        tf.test("空函数生成", ir.instructions.size() > 0);
        tf.test("空函数包含标签", hasInstruction(ir, "label", "main"));
    }
}

// 测试算术表达式
void testArithmeticExpressions(TestFramework &tf) {
    tf.startSuite("算术表达式测试");

    // 测试加法
    {
        auto ir = generateIR("int main() { return 1 + 2; }");
        tf.test("加法表达式生成", hasInstruction(ir, "add"));
        tf.test("加法操作数正确", countInstructions(ir, "mov") >= 2); // 至少两个数字
    }

    // 测试减法
    {
        auto ir = generateIR("int main() { return 5 - 3; }");
        tf.test("减法表达式生成", hasInstruction(ir, "sub"));
    }

    // 测试乘法
    {
        auto ir = generateIR("int main() { return 3 * 4; }");
        tf.test("乘法表达式生成", hasInstruction(ir, "mul"));
    }

    // 测试除法
    {
        auto ir = generateIR("int main() { return 8 / 2; }");
        tf.test("除法表达式生成", hasInstruction(ir, "div"));
    }

    // 测试复合算术表达式
    {
        auto ir = generateIR("int main() { return 1 + 2 * 3; }");
        tf.test("复合表达式包含加法", hasInstruction(ir, "add"));
        tf.test("复合表达式包含乘法", hasInstruction(ir, "mul"));
        tf.test("复合表达式有足够的临时变量", countInstructions(ir, "mov") >= 3);
    }
}

// 测试比较表达式
void testComparisonExpressions(TestFramework &tf) {
    tf.startSuite("比较表达式测试");

    // 测试小于
    {
        auto ir = generateIR("int main() { return 1 < 2; }");
        tf.test("小于比较生成", hasInstruction(ir, "lt"));
    }

    // 测试小于等于
    {
        auto ir = generateIR("int main() { return 1 <= 2; }");
        tf.test("小于等于比较生成", hasInstruction(ir, "le"));
    }

    // 测试大于
    {
        auto ir = generateIR("int main() { return 2 > 1; }");
        tf.test("大于比较生成", hasInstruction(ir, "gt"));
    }

    // 测试大于等于
    {
        auto ir = generateIR("int main() { return 2 >= 1; }");
        tf.test("大于等于比较生成", hasInstruction(ir, "ge"));
    }

    // 测试等于
    {
        auto ir = generateIR("int main() { return 1 == 1; }");
        tf.test("等于比较生成", hasInstruction(ir, "eq"));
    }

    // 测试不等于
    {
        auto ir = generateIR("int main() { return 1 != 2; }");
        tf.test("不等于比较生成", hasInstruction(ir, "ne"));
    }
}

// 测试一元表达式
void testUnaryExpressions(TestFramework &tf) {
    tf.startSuite("一元表达式测试");

    // 测试负号
    {
        auto ir = generateIR("int main() { return -5; }");
        tf.test("负号表达式生成", hasInstruction(ir, "neg"));
    }

    // 测试逻辑非（虽然当前语法可能不支持，但测试框架准备好了）
    // 注意：检查实际语法支持情况
}

// 测试变量声明和赋值
void testVariablesAndAssignment(TestFramework &tf) {
    tf.startSuite("变量和赋值测试");

    // 测试变量声明与初始化
    {
        auto ir = generateIR("int main() { int x = 5; return x; }");
        tf.test("变量初始化生成mov指令", hasInstruction(ir, "mov", "x"));
    }

    // 测试变量赋值
    {
        auto ir = generateIR("int main() { int x; x = 10; return x; }");
        tf.test("变量赋值生成mov指令", hasInstruction(ir, "mov", "x"));
    }

    // 测试变量在表达式中的使用
    {
        auto ir = generateIR("int main() { int x = 5; int y = 3; return x + y; }");
        tf.test("变量表达式生成加法", hasInstruction(ir, "add"));
        tf.test("包含变量初始化", hasInstruction(ir, "mov", "x"));
        tf.test("包含变量初始化", hasInstruction(ir, "mov", "y"));
    }

    // 测试赋值表达式
    {
        auto ir = generateIR("int main() { int x; x = 5 + 3; return x; }");
        tf.test("赋值表达式包含加法", hasInstruction(ir, "add"));
        tf.test("赋值表达式包含mov", hasInstruction(ir, "mov", "x"));
    }
}

// 测试控制流
void testControlFlow(TestFramework &tf) {
    tf.startSuite("控制流测试");

    // 测试if语句
    {
        auto ir = generateIR("int main() { if (1 > 0) return 1; return 0; }");
        tf.test("if语句包含比较", hasInstruction(ir, "gt"));
        tf.test("if语句包含条件跳转", hasInstruction(ir, "brz"));
        tf.test("if语句包含标签", hasInstruction(ir, "label"));
        tf.test("if语句包含无条件跳转", hasInstruction(ir, "jmp"));
    }

    // 测试if-else语句
    {
        auto ir = generateIR("int main() { if (1 > 0) return 1; else return 2; }");
        tf.test("if-else包含比较", hasInstruction(ir, "gt"));
        tf.test("if-else包含条件跳转", hasInstruction(ir, "brz"));
        tf.test("if-else至少有两个标签", countInstructions(ir, "label") >= 3); // main + else + end
    }

    // 测试while循环
    {
        auto ir = generateIR("int main() { int x = 5; while (x > 0) x = x - 1; return x; }");
        tf.test("while循环包含比较", hasInstruction(ir, "gt"));
        tf.test("while循环包含条件跳转", hasInstruction(ir, "brz"));
        tf.test("while循环包含无条件跳转", hasInstruction(ir, "jmp"));
        tf.test("while循环至少有两个标签", countInstructions(ir, "label") >= 3); // main + start + end
        tf.test("while循环包含减法", hasInstruction(ir, "sub"));
    }

    // 测试嵌套控制流
    {
        auto ir = generateIR(R"(
            int main() {
                int x = 10;
                while (x > 0) {
                    if (x > 5)
                        x = x - 2;
                    else
                        x = x - 1;
                }
                return x;
            }
        )");
        tf.test("嵌套控制流包含多个比较", countInstructions(ir, "gt") >= 2);
        tf.test("嵌套控制流包含多个条件跳转", countInstructions(ir, "brz") >= 2);
        tf.test("嵌套控制流包含多个标签", countInstructions(ir, "label") >= 4);
    }
}

// 测试函数调用
void testFunctionCalls(TestFramework &tf) {
    tf.startSuite("函数调用测试");

    // 测试无参数函数调用
    {
        auto ir = generateIR(R"(
            int foo() { return 42; }
            int main() { return foo(); }
        )");
        tf.test("无参函数调用生成call指令", hasInstruction(ir, "call"));
        tf.test("包含两个函数标签", hasInstruction(ir, "label", "foo") && hasInstruction(ir, "label", "main"));
    }

    // 测试带参数函数调用
    {
        auto ir = generateIR(R"(
            int add(int a, int b) { return a + b; }
            int main() { return add(1, 2); }
        )");
        tf.test("带参函数调用生成call指令", hasInstruction(ir, "call"));
        tf.test("包含加法运算", hasInstruction(ir, "add"));
    }

    // 测试嵌套函数调用
    {
        auto ir = generateIR(R"(
            int double_val(int x) { return x * 2; }
            int add(int a, int b) { return a + b; }
            int main() { return add(double_val(3), double_val(4)); }
        )");
        tf.test("嵌套调用包含多个call", countInstructions(ir, "call") >= 3); // 2个double_val + 1个add
        tf.test("嵌套调用包含乘法", hasInstruction(ir, "mul"));
        tf.test("嵌套调用包含加法", hasInstruction(ir, "add"));
    }
}

// 测试复杂表达式
void testComplexExpressions(TestFramework &tf) {
    tf.startSuite("复杂表达式测试");

    // 测试多层嵌套表达式
    {
        auto ir = generateIR("int main() { return (1 + 2) * (3 - 4); }");
        tf.test("嵌套表达式包含加法", hasInstruction(ir, "add"));
        tf.test("嵌套表达式包含减法", hasInstruction(ir, "sub"));
        tf.test("嵌套表达式包含乘法", hasInstruction(ir, "mul"));
        tf.test("嵌套表达式生成足够临时变量", countInstructions(ir, "mov") >= 4);
    }

    // 测试混合运算和比较
    {
        auto ir = generateIR("int main() { return (1 + 2) > (3 * 4); }");
        tf.test("混合运算包含加法", hasInstruction(ir, "add"));
        tf.test("混合运算包含乘法", hasInstruction(ir, "mul"));
        tf.test("混合运算包含比较", hasInstruction(ir, "gt"));
    }

    // 测试多变量复杂表达式
    {
        auto ir = generateIR(R"(
            int main() {
                int a = 1;
                int b = 2;
                int c = 3;
                return a * b + c * (a - b);
            }
        )");
        tf.test("多变量表达式包含乘法", countInstructions(ir, "mul") >= 2);
        tf.test("多变量表达式包含加法", hasInstruction(ir, "add"));
        tf.test("多变量表达式包含减法", hasInstruction(ir, "sub"));
    }
}

// 测试IR指令顺序和正确性
void testIRCorrectness(TestFramework &tf) {
    tf.startSuite("IR正确性测试");

    // 测试函数标签位置
    {
        auto ir = generateIR("int main() { return 0; }");
        int labelPos = findInstruction(ir, "label", "main");
        tf.test("函数标签在开头", labelPos == 0);
    }

    // 测试临时变量生成
    {
        auto ir = generateIR("int main() { return 1 + 2 + 3; }");
        bool hasTemp = false;
        for (const auto &inst : ir.instructions) {
            if (inst.dst.find("t") == 0 || inst.src1.find("t") == 0 || inst.src2.find("t") == 0) {
                hasTemp = true;
                break;
            }
        }
        tf.test("生成临时变量", hasTemp);
    }

    // 测试条件跳转目标
    {
        auto ir = generateIR("int main() { if (1 > 0) return 1; return 0; }");
        bool hasValidBranch = false;
        bool hasValidLabel = false;
        
        for (const auto &inst : ir.instructions) {
            if (inst.op == "brz" && inst.src1.find("L") == 0) {
                hasValidBranch = true;
            }
            if (inst.op == "label" && inst.dst.find("L") == 0) {
                hasValidLabel = true;
            }
        }
        tf.test("条件跳转目标格式正确", hasValidBranch);
        tf.test("标签格式正确", hasValidLabel);
    }

    // 测试return指令的操作数
    {
        auto ir = generateIR("int main() { return 42; }");
        bool hasReturnWithOperand = false;
        for (const auto &inst : ir.instructions) {
            if (inst.op == "ret" && !inst.dst.empty()) {
                hasReturnWithOperand = true;
                break;
            }
        }
        tf.test("return指令有操作数", hasReturnWithOperand);
    }
}

// 测试边界情况
void testEdgeCases(TestFramework &tf) {
    tf.startSuite("边界情况测试");

    // 测试多个函数
    {
        auto ir = generateIR(R"(
            int func1() { return 1; }
            int func2() { return 2; }
            int main() { return func1() + func2(); }
        )");
        tf.test("多函数包含所有标签", 
                hasInstruction(ir, "label", "func1") && 
                hasInstruction(ir, "label", "func2") && 
                hasInstruction(ir, "label", "main"));
        tf.test("多函数包含函数调用", countInstructions(ir, "call") >= 2);
    }

    // 测试深度嵌套块
    {
        auto ir = generateIR(R"(
            int main() {
                {
                    {
                        int x = 1;
                        return x;
                    }
                }
            }
        )");
        tf.test("嵌套块正常生成", hasInstruction(ir, "mov", "x"));
    }

    // 测试空return
    {
        auto ir = generateIR("int main() { return; }");
        bool hasEmptyReturn = false;
        for (const auto &inst : ir.instructions) {
            if (inst.op == "ret" && inst.src1.empty()) {
                hasEmptyReturn = true;
                break;
            }
        }
        tf.test("空return指令正确", hasEmptyReturn);
    }

    // 测试单个表达式语句
    {
        auto ir = generateIR("int main() { int x; x = 5; }");
        tf.test("表达式语句生成IR", hasInstruction(ir, "mov", "x"));
    }
}

// 测试错误处理
void testErrorHandling(TestFramework &tf) {
    tf.startSuite("错误处理测试");

    // 测试无效AST节点类型
    {
        bool caughtError = false;
        try {
            // 创建一个无效的AST节点进行测试
            auto ast = std::make_unique<ASTProgram>();
            // 模拟一个错误：传递非Program节点给generateIR
            ASTFunction fakeRoot;
            IRBuilder builder;
            builder.generateIR(&fakeRoot);
        } catch (const std::runtime_error &e) {
            caughtError = true;
        } catch (...) {
            caughtError = true;
        }
        tf.test("无效根节点抛出异常", caughtError);
    }
}

// 主测试函数
int main() {
    TestFramework tf;
    
    std::cout << "MIPS 编译器 IRBuilder 单元测试" << std::endl;
    std::cout << "==================================" << std::endl;

    // 运行所有测试
    testBasicIRGeneration(tf);
    testArithmeticExpressions(tf);
    testComparisonExpressions(tf);
    testUnaryExpressions(tf);
    testVariablesAndAssignment(tf);
    testControlFlow(tf);
    testFunctionCalls(tf);
    testComplexExpressions(tf);
    testIRCorrectness(tf);
    testEdgeCases(tf);
    testErrorHandling(tf);

    // 显示测试结果
    tf.summary();

    return tf.allPassed() ? 0 : 1;
}
