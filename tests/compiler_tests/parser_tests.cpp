/*
test_parser.cpp: MIPS 编译器语法分析器单元测试
作用：测试 Parser 类和 AST 构建功能的正确性
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

// 测试基础表达式解析
void testBasicExpressions(TestFramework &tf) {
    tf.startSuite("基础表达式测试");

    // 测试数字表达式
    {
        auto ast = parseCode("int main() { return 42; }");
        tf.test("数字表达式解析", ast->functions.size() == 1);
        
        auto func = ast->functions[0].get();
        tf.test("函数名正确", func->name == "main");
        
        auto block = func->body.get();
        tf.test("函数体存在", block != nullptr && block->statements.size() == 1);
        
        if (block && block->statements.size() == 1) {
            auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
            tf.test("返回语句类型正确", ret != nullptr);
            
            if (ret && ret->value) {
                auto num = dynamic_cast<ASTNumberExpr*>(ret->value.get());
                tf.test("数字节点类型正确", num != nullptr);
                tf.test("数字值正确", num && num->value == 42);
            }
        }
    }

    // 测试标识符表达式
    {
        auto ast = parseCode("int main() { int x; return x; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        
        tf.test("两个语句存在", block->statements.size() == 2);
        
        if (block->statements.size() == 2) {
            auto ret = dynamic_cast<ASTReturn*>(block->statements[1].get());
            if (ret && ret->value) {
                auto id = dynamic_cast<ASTIdentifierExpr*>(ret->value.get());
                tf.test("标识符节点类型正确", id != nullptr);
                tf.test("标识符名称正确", id && id->name == "x");
            }
        }
    }
}

// 测试二元表达式
void testBinaryExpressions(TestFramework &tf) {
    tf.startSuite("二元表达式测试");

    // 测试加法表达式
    {
        auto ast = parseCode("int main() { return 1 + 2; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto bin = dynamic_cast<ASTBinaryExpr*>(ret->value.get());
            tf.test("二元表达式节点正确", bin != nullptr);
            tf.test("加法操作符正确", bin && bin->op == ASTBinaryOpKind::Add);
            
            if (bin) {
                auto left = dynamic_cast<ASTNumberExpr*>(bin->left.get());
                auto right = dynamic_cast<ASTNumberExpr*>(bin->right.get());
                tf.test("左操作数正确", left && left->value == 1);
                tf.test("右操作数正确", right && right->value == 2);
            }
        }
    }

    // 测试优先级（乘法比加法优先级高）
    {
        auto ast = parseCode("int main() { return 1 + 2 * 3; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto bin = dynamic_cast<ASTBinaryExpr*>(ret->value.get());
            tf.test("外层是加法", bin && bin->op == ASTBinaryOpKind::Add);
            
            if (bin) {
                auto left = dynamic_cast<ASTNumberExpr*>(bin->left.get());
                auto right = dynamic_cast<ASTBinaryExpr*>(bin->right.get());
                tf.test("左边是数字1", left && left->value == 1);
                tf.test("右边是乘法表达式", right && right->op == ASTBinaryOpKind::Mul);
            }
        }
    }

    // 测试比较表达式
    {
        auto ast = parseCode("int main() { return x < y; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto bin = dynamic_cast<ASTBinaryExpr*>(ret->value.get());
            tf.test("比较表达式正确", bin && bin->op == ASTBinaryOpKind::Less);
        }
    }
}

// 测试一元表达式
void testUnaryExpressions(TestFramework &tf) {
    tf.startSuite("一元表达式测试");

    // 测试负数
    {
        auto ast = parseCode("int main() { return -5; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto un = dynamic_cast<ASTUnaryExpr*>(ret->value.get());
            tf.test("一元表达式节点正确", un != nullptr);
            tf.test("负号操作符正确", un && un->op == ASTUnaryOpKind::Neg);
            
            if (un && un->expr) {
                auto num = dynamic_cast<ASTNumberExpr*>(un->expr.get());
                tf.test("操作数是数字5", num && num->value == 5);
            }
        }
    }
}

// 测试变量声明
void testVariableDeclarations(TestFramework &tf) {
    tf.startSuite("变量声明测试");

    // 测试简单声明
    {
        auto ast = parseCode("int main() { int x; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        
        tf.test("有一个语句", block->statements.size() == 1);
        
        if (block->statements.size() == 1) {
            auto decl = dynamic_cast<ASTVarDecl*>(block->statements[0].get());
            tf.test("变量声明节点正确", decl != nullptr);
            tf.test("变量类型正确", decl && decl->typeName == "int");
            tf.test("变量名称正确", decl && decl->name == "x");
            tf.test("无初始化表达式", decl && decl->initExpr == nullptr);
        }
    }

    // 测试带初始化的声明
    {
        auto ast = parseCode("int main() { int x = 10; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto decl = dynamic_cast<ASTVarDecl*>(block->statements[0].get());
        
        tf.test("有初始化表达式", decl && decl->initExpr != nullptr);
        
        if (decl && decl->initExpr) {
            auto num = dynamic_cast<ASTNumberExpr*>(decl->initExpr.get());
            tf.test("初始化值正确", num && num->value == 10);
        }
    }
}

// 测试赋值表达式
void testAssignmentExpressions(TestFramework &tf) {
    tf.startSuite("赋值表达式测试");

    auto ast = parseCode("int main() { x = 5; }");
    auto func = ast->functions[0].get();
    auto block = func->body.get();
    auto exprStmt = dynamic_cast<ASTExprStmt*>(block->statements[0].get());
    
    tf.test("表达式语句存在", exprStmt != nullptr);
    
    if (exprStmt && exprStmt->expr) {
        auto assign = dynamic_cast<ASTAssignExpr*>(exprStmt->expr.get());
        tf.test("赋值表达式节点正确", assign != nullptr);
        tf.test("赋值目标正确", assign && assign->name == "x");
        
        if (assign && assign->value) {
            auto num = dynamic_cast<ASTNumberExpr*>(assign->value.get());
            tf.test("赋值值正确", num && num->value == 5);
        }
    }
}

// 测试控制流语句
void testControlFlow(TestFramework &tf) {
    tf.startSuite("控制流测试");

    // 测试if语句
    {
        auto ast = parseCode("int main() { if (x > 0) return 1; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ifStmt = dynamic_cast<ASTIf*>(block->statements[0].get());
        
        tf.test("if语句节点正确", ifStmt != nullptr);
        tf.test("有条件表达式", ifStmt && ifStmt->condition != nullptr);
        tf.test("有then分支", ifStmt && ifStmt->thenBranch != nullptr);
        tf.test("无else分支", ifStmt && ifStmt->elseBranch == nullptr);
    }

    // 测试if-else语句
    {
        auto ast = parseCode("int main() { if (x > 0) return 1; else return 0; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ifStmt = dynamic_cast<ASTIf*>(block->statements[0].get());
        
        tf.test("if-else语句正确", ifStmt != nullptr);
        tf.test("有else分支", ifStmt && ifStmt->elseBranch != nullptr);
    }

    // 测试while循环
    {
        auto ast = parseCode("int main() { while (x > 0) x = x - 1; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto whileStmt = dynamic_cast<ASTWhile*>(block->statements[0].get());
        
        tf.test("while语句节点正确", whileStmt != nullptr);
        tf.test("有条件表达式", whileStmt && whileStmt->condition != nullptr);
        tf.test("有循环体", whileStmt && whileStmt->body != nullptr);
    }
}

// 测试函数定义
void testFunctionDefinitions(TestFramework &tf) {
    tf.startSuite("函数定义测试");

    // 测试无参数函数
    {
        auto ast = parseCode("int main() { return 0; }");
        tf.test("有一个函数", ast->functions.size() == 1);
        
        auto func = ast->functions[0].get();
        tf.test("函数返回类型正确", func && func->returnType == "int");
        tf.test("函数名正确", func && func->name == "main");
        tf.test("无参数", func && func->paramNames.empty());
    }

    // 测试带参数函数
    {
        auto ast = parseCode("int add(int a, int b) { return a + b; }");
        auto func = ast->functions[0].get();
        
        tf.test("函数名正确", func && func->name == "add");
        tf.test("有两个参数", func && func->paramNames.size() == 2);
        
        if (func && func->paramNames.size() == 2) {
            tf.test("第一个参数名正确", func->paramNames[0] == "a");
            tf.test("第二个参数名正确", func->paramNames[1] == "b");
            tf.test("参数类型都是int", func->paramTypes[0] == "int" && func->paramTypes[1] == "int");
        }
    }
}

// 测试函数调用
void testFunctionCalls(TestFramework &tf) {
    tf.startSuite("函数调用测试");

    // 测试无参数函数调用
    {
        auto ast = parseCode("int main() { return foo(); }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto call = dynamic_cast<ASTCallExpr*>(ret->value.get());
            tf.test("函数调用节点正确", call != nullptr);
            tf.test("函数名正确", call && call->callee == "foo");
            tf.test("无参数", call && call->arguments.empty());
        }
    }

    // 测试带参数函数调用
    {
        auto ast = parseCode("int main() { return add(1, 2); }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto call = dynamic_cast<ASTCallExpr*>(ret->value.get());
            tf.test("函数调用节点正确", call != nullptr);
            tf.test("函数名正确", call && call->callee == "add");
            tf.test("有两个参数", call && call->arguments.size() == 2);
        }
    }
}

// 测试复杂表达式
void testComplexExpressions(TestFramework &tf) {
    tf.startSuite("复杂表达式测试");

    // 测试括号表达式
    {
        auto ast = parseCode("int main() { return (1 + 2) * 3; }");
        auto func = ast->functions[0].get();
        auto block = func->body.get();
        auto ret = dynamic_cast<ASTReturn*>(block->statements[0].get());
        
        if (ret && ret->value) {
            auto bin = dynamic_cast<ASTBinaryExpr*>(ret->value.get());
            tf.test("外层是乘法", bin && bin->op == ASTBinaryOpKind::Mul);
            
            if (bin && bin->left) {
                auto leftBin = dynamic_cast<ASTBinaryExpr*>(bin->left.get());
                tf.test("左边是加法（括号改变优先级）", leftBin && leftBin->op == ASTBinaryOpKind::Add);
            }
        }
    }
}

// 测试AST dump功能
void testASTDump(TestFramework &tf) {
    tf.startSuite("AST Dump测试");

    auto ast = parseCode("int main() { return 42; }");
    
    // 测试dump不会崩溃
    std::stringstream ss;
    try {
        dumpAST(*ast, ss);
        tf.test("AST dump执行成功", true);
        
        std::string output = ss.str();
        tf.test("输出包含Program", output.find("Program") != std::string::npos);
        tf.test("输出包含Function", output.find("Function") != std::string::npos);
        tf.test("输出包含main", output.find("main") != std::string::npos);
    } catch (...) {
        tf.test("AST dump执行成功", false);
    }
}

// 测试错误处理
void testErrorHandling(TestFramework &tf) {
    tf.startSuite("错误处理测试");

    // 测试语法错误
    {
        Lexer lexer("int main( { return 0; }"); // 缺少右括号
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        
        bool caught = false;
        try {
            parser.parseProgram();
        } catch (const ParseError &e) {
            caught = true;
        } catch (...) {
            // 其他异常也算测试通过，因为确实检测到了错误
            caught = true;
        }
        tf.test("语法错误被正确捕获", caught);
    }
}

// 主测试函数
int main() {
    TestFramework tf;
    
    std::cout << "MIPS 编译器语法分析器单元测试" << std::endl;
    std::cout << "================================" << std::endl;

    // 运行所有测试
    testBasicExpressions(tf);
    testBinaryExpressions(tf);
    testUnaryExpressions(tf);
    testVariableDeclarations(tf);
    testAssignmentExpressions(tf);
    testControlFlow(tf);
    testFunctionDefinitions(tf);
    testFunctionCalls(tf);
    testComplexExpressions(tf);
    testASTDump(tf);
    testErrorHandling(tf);

    // 显示测试结果
    tf.summary();

    return tf.allPassed() ? 0 : 1;
}
