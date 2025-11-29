/*
semantic_analyzer_tests.cpp: 语义分析器单元测试
作用：测试语义分析器和符号表功能的正确性
*/

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <vector>

// 引入要测试的模块
#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include "compiler/ast.hpp"
#include "compiler/symbol_table.hpp"
#include "compiler/semantic_analyzer.hpp"

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

// 辅助函数：解析代码并返回AST
std::unique_ptr<ASTProgram> parseCode(const std::string &code) {
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    return parser.parseProgram();
}

// 辅助函数：执行语义分析（成功情况）
bool analyzeCode(const std::string &code) {
    try {
        auto ast = parseCode(code);
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        return true;
    } catch (const SemanticError &e) {
        // std::cout << "SemanticError: " << e.what() << std::endl; // debug
        return false;
    } catch (const ParseError &e) {
        // std::cout << "ParseError: " << e.what() << std::endl; // debug
        return false;
    } catch (const std::exception &e) {
        // std::cout << "Other Error: " << e.what() << std::endl; // debug
        return false;
    } catch (...) {
        // std::cout << "Unknown Error" << std::endl; // debug
        return false;
    }
}

// 辅助函数：执行语义分析并期望错误
bool expectSemanticError(const std::string &code, const std::string &expectedErrorSubstr = "") {
    try {
        auto ast = parseCode(code);
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        return false; // 没有抛出异常，测试失败
    } catch (const SemanticError &e) {
        if (expectedErrorSubstr.empty()) {
            return true; // 只要抛出语义错误就算成功
        }
        std::string errorMsg = e.what();
        return errorMsg.find(expectedErrorSubstr) != std::string::npos;
    } catch (...) {
        return false; // 其他异常，测试失败
    }
}

// 测试符号表基础功能
void testSymbolTableBasics(TestFramework &tf) {
    tf.startSuite("符号表基础功能测试");

    // 测试基本的声明和查找
    {
        SymbolTable st;
        st.enterScope();
        
        tf.test("声明新变量成功", st.declare("x", "int"));
        tf.test("查找已声明变量", st.lookup("x") == "int");
        tf.test("查找未声明变量", st.lookup("y") == "");
        tf.test("重复声明失败", !st.declare("x", "int"));
        
        st.exitScope();
    }

    // 测试多级作用域
    {
        SymbolTable st;
        st.enterScope(); // global scope
        
        st.declare("global_var", "int");
        tf.test("全局变量可查找", st.lookup("global_var") == "int");
        
        st.enterScope(); // local scope
        st.declare("local_var", "int");
        
        tf.test("局部变量可查找", st.lookup("local_var") == "int");
        tf.test("全局变量仍可查找", st.lookup("global_var") == "int");
        
        st.exitScope(); // back to global
        
        tf.test("退出局部作用域后，局部变量不可见", st.lookup("local_var") == "");
        tf.test("退出局部作用域后，全局变量仍可见", st.lookup("global_var") == "int");
        
        st.exitScope();
    }

    // 测试变量遮蔽（shadowing）
    {
        SymbolTable st;
        st.enterScope(); // global
        st.declare("x", "int");
        
        st.enterScope(); // local
        st.declare("x", "int"); // 遮蔽全局变量
        
        tf.test("局部变量遮蔽全局变量", st.lookup("x") == "int");
        
        st.exitScope();
        tf.test("退出作用域后，全局变量恢复可见", st.lookup("x") == "int");
        
        st.exitScope();
    }

    // 测试异常情况
    {
        SymbolTable st;
        bool caught = false;
        
        try {
            st.declare("x", "int"); // 无作用域时声明
        } catch (const std::runtime_error &e) {
            caught = true;
        }
        tf.test("无作用域时声明抛出异常", caught);
        
        caught = false;
        try {
            st.exitScope(); // 空作用域栈时退出
        } catch (const std::runtime_error &e) {
            caught = true;
        }
        tf.test("空作用域栈时退出抛出异常", caught);
    }
}

// 测试成功的语义分析案例
void testValidSemantics(TestFramework &tf) {
    tf.startSuite("有效语义测试");

    // 测试简单函数
    {
        std::string code = "int main() { return 42; }";
        tf.test("简单函数语义正确", analyzeCode(code));
    }

    // 测试变量声明和使用
    {
        std::string code = "int main() { int x; x = 5; return x; }";
        tf.test("变量声明和使用", analyzeCode(code));
    }

    // 测试带初始化的变量声明
    {
        std::string code = "int main() { int x = 10; return x; }";
        tf.test("带初始化的变量声明", analyzeCode(code));
    }

    // 测试算术表达式
    {
        std::string code = "int main() { int x = 1; int y = 2; return x + y * 3; }";
        tf.test("算术表达式", analyzeCode(code));
    }

    // 测试比较表达式
    {
        std::string code = "int main() { int x = 5; int y = 10; return x < y; }";
        tf.test("比较表达式", analyzeCode(code));
    }

    // 测试一元表达式
    {
        std::string code = "int main() { int x = 5; return -x; }";
        tf.test("一元表达式", analyzeCode(code));
    }

    // 测试if语句
    {
        std::string code = "int main() { int x = 5; if (x > 0) return 1; return 0; }";
        tf.test("if语句", analyzeCode(code));
    }

    // 测试if-else语句
    {
        std::string code = "int main() { int x = 5; if (x > 0) return 1; else return -1; }";
        tf.test("if-else语句", analyzeCode(code));
    }

    // 测试while循环
    {
        std::string code = "int main() { int x = 5; while (x > 0) x = x - 1; return x; }";
        tf.test("while循环", analyzeCode(code));
    }

    // 测试嵌套块作用域
    {
        std::string code = R"(
            int main() {
                int x = 1;
                {
                    int y = 2;
                    x = x + y;
                }
                return x;
            }
        )";
        tf.test("嵌套块作用域", analyzeCode(code));
    }

    // 测试函数参数
    {
        std::string code = "int add(int a, int b) { return a + b; }";
        tf.test("带参数的函数", analyzeCode(code));
    }

    // 测试函数调用
    {
        std::string code = R"(
            int square(int x) { return x * x; }
            int main() { return square(5); }
        )";
        tf.test("函数调用", analyzeCode(code));
    }

    // 测试多个函数
    {
        std::string code = R"(
            int add(int a, int b) { return a + b; }
            int multiply(int a, int b) { return a * b; }
            int main() { return add(2, multiply(3, 4)); }
        )";
        tf.test("多个函数和嵌套调用", analyzeCode(code));
    }
}

// 测试变量相关的语义错误
void testVariableSemanticErrors(TestFramework &tf) {
    tf.startSuite("变量语义错误测试");

    // 测试使用未声明的变量
    {
        std::string code = "int main() { return x; }";
        tf.test("使用未声明变量", expectSemanticError(code, "undeclared variable"));
    }

    // 测试变量重复声明
    {
        std::string code = "int main() { int x; int x; return 0; }";
        tf.test("变量重复声明", expectSemanticError(code, "redeclared"));
    }

    // 测试赋值给未声明变量
    {
        std::string code = "int main() { x = 5; return 0; }";
        tf.test("赋值给未声明变量", expectSemanticError(code, "undeclared variable"));
    }

    // 测试参数重复声明
    {
        std::string code = "int func(int x, int x) { return 0; }";
        tf.test("参数重复声明", expectSemanticError(code, "Duplicate parameter"));
    }

    // 测试使用超出作用域的变量
    {
        std::string code = R"(
            int main() {
                {
                    int x = 5;
                }
                return x;
            }
        )";
        tf.test("使用超出作用域的变量", expectSemanticError(code, "undeclared variable"));
    }
}

// 测试函数相关的语义错误
void testFunctionSemanticErrors(TestFramework &tf) {
    tf.startSuite("函数语义错误测试");

    // 测试函数重复定义
    {
        std::string code = R"(
            int func() { return 1; }
            int func() { return 2; }
        )";
        tf.test("函数重复定义", expectSemanticError(code, "Redefinition of function"));
    }

    // 测试调用未声明的函数
    {
        std::string code = "int main() { return foo(); }";
        tf.test("调用未声明的函数", expectSemanticError(code, "undeclared function"));
    }

    // 测试函数名与变量名冲突
    {
        std::string code = R"(
            int main() { return 0; }
            int main2() {
                int main = 5;
                return main;
            }
        )";
        tf.test("变量遮蔽函数名", analyzeCode(code));
    }
}

// 测试类型相关的语义错误
void testTypeSemanticErrors(TestFramework &tf) {
    tf.startSuite("类型语义错误测试");

    // 注意：由于当前实现只支持int类型，这里的测试相对简单
    // 主要测试表达式类型检查

    // 测试条件语句类型错误（虽然当前所有表达式都返回int，但测试框架准备好了）
    {
        std::string code = "int main() { int x = 5; if (x) return 1; return 0; }";
        tf.test("if条件类型检查", analyzeCode(code)); // 应该通过，因为x是int
    }

    // 测试while条件类型检查
    {
        std::string code = "int main() { int x = 5; while (x > 0) x = x - 1; return 0; }";
        tf.test("while条件类型检查", analyzeCode(code)); // 应该通过
    }

    // 测试返回值类型检查
    {
        std::string code = "int main() { int x = 5; return x; }";
        tf.test("返回值类型检查", analyzeCode(code)); // 应该通过
    }
}

// 测试复杂的语义分析场景
void testComplexSemantics(TestFramework &tf) {
    tf.startSuite("复杂语义测试");

    // 测试嵌套作用域中的变量使用
    {
        std::string code = "int main() { int x = 1; { int y = 2; x = 3; } return x; }";
        tf.test("嵌套作用域变量使用", analyzeCode(code));
    }

    // 测试递归函数
    {
        std::string code = R"(
            int factorial(int n) {
                if (n <= 1)
                    return 1;
                else
                    return n * factorial(n - 1);
            }
            int main() {
                return factorial(5);
            }
        )";
        tf.test("递归函数调用", analyzeCode(code));
    }

    // 测试复杂的表达式嵌套
    {
        std::string code = R"(
            int compute(int a, int b) {
                return a * b + 1;
            }
            int main() {
                int x = 2;
                int y = 3;
                int result = compute(x + 1, y * 2) - compute(x, y);
                return result;
            }
        )";
        tf.test("复杂表达式嵌套", analyzeCode(code));
    }

    // 测试多层嵌套的控制结构
    {
        std::string code = R"(
            int main() {
                int i = 0;
                int sum = 0;
                while (i < 10) {
                    if (i > 5) {
                        sum = sum + i;
                    } else {
                        sum = sum - i;
                    }
                    i = i + 1;
                }
                return sum;
            }
        )";
        tf.test("多层嵌套控制结构", analyzeCode(code));
    }

    // 测试函数调用作为其他函数的参数
    {
        std::string code = R"(
            int add(int a, int b) { return a + b; }
            int multiply(int a, int b) { return a * b; }
            int compute(int x) { return x * 2; }
            int main() {
                return add(multiply(2, 3), compute(4));
            }
        )";
        tf.test("函数调用作为参数", analyzeCode(code));
    }
}

// 测试边界情况和异常处理
void testEdgeCases(TestFramework &tf) {
    tf.startSuite("边界情况测试");

    // 测试空函数
    {
        std::string code = "int main() { }";
        tf.test("空函数体", analyzeCode(code));
    }

    // 测试只有return的函数
    {
        std::string code = "int main() { return 0; }";
        tf.test("只有return的函数", analyzeCode(code));
    }

    // 测试空return
    {
        std::string code = "int main() { return; }";
        tf.test("空return语句", analyzeCode(code));
    }

    // 测试大量变量声明
    {
        std::string code = R"(
            int main() {
                int a, b, c, d, e, f, g, h, i, j;
                a = 1; b = 2; c = 3; d = 4; e = 5;
                f = 6; g = 7; h = 8; i = 9; j = 10;
                return a + b + c + d + e + f + g + h + i + j;
            }
        )";
        // 注意：当前语法不支持int a, b;这样的声明，所以这个测试会失败
        // 这里我们修改为逐个声明
        std::string validCode = R"(
            int main() {
                int a; int b; int c; int d; int e;
                int f; int g; int h; int i; int j;
                a = 1; b = 2; c = 3; d = 4; e = 5;
                f = 6; g = 7; h = 8; i = 9; j = 10;
                return a + b + c + d + e + f + g + h + i + j;
            }
        )";
        tf.test("大量变量声明", analyzeCode(validCode));
    }

    // 测试深度嵌套的作用域
    {
        std::string code = R"(
            int main() {
                int x = 1;
                {
                    int y = 2;
                    {
                        int z = 3;
                        {
                            int w = 4;
                            x = x + y + z + w;
                        }
                    }
                }
                return x;
            }
        )";
        tf.test("深度嵌套作用域", analyzeCode(code));
    }
}

// 主测试函数
int main() {
    TestFramework tf;
    
    std::cout << "MIPS 编译器语义分析器单元测试" << std::endl;
    std::cout << "==================================" << std::endl;

    // 运行所有测试
    testSymbolTableBasics(tf);
    testValidSemantics(tf);
    testVariableSemanticErrors(tf);
    testFunctionSemanticErrors(tf);
    testTypeSemanticErrors(tf);
    testComplexSemantics(tf);
    testEdgeCases(tf);

    // 显示测试结果
    tf.summary();

    return tf.allPassed() ? 0 : 1;
}
