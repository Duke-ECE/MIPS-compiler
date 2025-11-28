#include "compiler/lexer.hpp"
#include "compiler/token.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// 辅助函数：检查token类型和文本
void assertToken(const Token& token, TokenType expectedType, const std::string& expectedText, int expectedLine = 1, int expectedCol = -1) {
    if (token.type != expectedType) {
        std::cerr << "Token type mismatch. Expected: " << static_cast<int>(expectedType) 
                  << ", Got: " << static_cast<int>(token.type) << std::endl;
        assert(false);
    }
    if (token.text != expectedText) {
        std::cerr << "Token text mismatch. Expected: '" << expectedText 
                  << "', Got: '" << token.text << "'" << std::endl;
        assert(false);
    }
    if (token.line != expectedLine) {
        std::cerr << "Token line mismatch. Expected: " << expectedLine 
                  << ", Got: " << token.line << std::endl;
        assert(false);
    }
    if (expectedCol != -1 && token.column != expectedCol) {
        std::cerr << "Token column mismatch. Expected: " << expectedCol 
                  << ", Got: " << token.column << std::endl;
        assert(false);
    }
}

// 测试基本关键字
void testKeywords() {
    std::cout << "Testing keywords..." << std::endl;
    
    Lexer lexer("int return if else while");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 6); // 5 keywords + EOF
    assertToken(tokens[0], TokenType::INT, "int");
    assertToken(tokens[1], TokenType::RETURN, "return");
    assertToken(tokens[2], TokenType::IF, "if");
    assertToken(tokens[3], TokenType::ELSE, "else");
    assertToken(tokens[4], TokenType::WHILE, "while");
    assertToken(tokens[5], TokenType::END_OF_FILE, "");
    
    std::cout << "Keywords test passed!" << std::endl;
}

// 测试标识符
void testIdentifiers() {
    std::cout << "Testing identifiers..." << std::endl;
    
    Lexer lexer("variable _underscore var123 _123abc");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 5); // 4 identifiers + EOF
    assertToken(tokens[0], TokenType::IDENTIFIER, "variable");
    assertToken(tokens[1], TokenType::IDENTIFIER, "_underscore");
    assertToken(tokens[2], TokenType::IDENTIFIER, "var123");
    assertToken(tokens[3], TokenType::IDENTIFIER, "_123abc");
    assertToken(tokens[4], TokenType::END_OF_FILE, "");
    
    std::cout << "Identifiers test passed!" << std::endl;
}

// 测试数字
void testNumbers() {
    std::cout << "Testing numbers..." << std::endl;
    
    Lexer lexer("0 123 456789");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 4); // 3 numbers + EOF
    assertToken(tokens[0], TokenType::NUMBER, "0");
    assertToken(tokens[1], TokenType::NUMBER, "123");
    assertToken(tokens[2], TokenType::NUMBER, "456789");
    assertToken(tokens[3], TokenType::END_OF_FILE, "");
    
    std::cout << "Numbers test passed!" << std::endl;
}

// 测试算术运算符
void testArithmeticOperators() {
    std::cout << "Testing arithmetic operators..." << std::endl;
    
    Lexer lexer("+ - * /");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 5); // 4 operators + EOF
    assertToken(tokens[0], TokenType::PLUS, "+");
    assertToken(tokens[1], TokenType::MINUS, "-");
    assertToken(tokens[2], TokenType::STAR, "*");
    assertToken(tokens[3], TokenType::SLASH, "/");
    assertToken(tokens[4], TokenType::END_OF_FILE, "");
    
    std::cout << "Arithmetic operators test passed!" << std::endl;
}

// 测试比较运算符
void testComparisonOperators() {
    std::cout << "Testing comparison operators..." << std::endl;
    
    Lexer lexer("== != < > <= >=");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 7); // 6 operators + EOF
    assertToken(tokens[0], TokenType::EQ, "==");
    assertToken(tokens[1], TokenType::NEQ, "!=");
    assertToken(tokens[2], TokenType::LT, "<");
    assertToken(tokens[3], TokenType::GT, ">");
    assertToken(tokens[4], TokenType::LE, "<=");
    assertToken(tokens[5], TokenType::GE, ">=");
    assertToken(tokens[6], TokenType::END_OF_FILE, "");
    
    std::cout << "Comparison operators test passed!" << std::endl;
}

// 测试赋值运算符
void testAssignmentOperator() {
    std::cout << "Testing assignment operator..." << std::endl;
    
    Lexer lexer("=");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 2); // 1 operator + EOF
    assertToken(tokens[0], TokenType::ASSIGN, "=");
    assertToken(tokens[1], TokenType::END_OF_FILE, "");
    
    std::cout << "Assignment operator test passed!" << std::endl;
}

// 测试分隔符
void testDelimiters() {
    std::cout << "Testing delimiters..." << std::endl;
    
    Lexer lexer("( ) { } ; ,");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 7); // 6 delimiters + EOF
    assertToken(tokens[0], TokenType::LPAREN, "(");
    assertToken(tokens[1], TokenType::RPAREN, ")");
    assertToken(tokens[2], TokenType::LBRACE, "{");
    assertToken(tokens[3], TokenType::RBRACE, "}");
    assertToken(tokens[4], TokenType::SEMICOLON, ";");
    assertToken(tokens[5], TokenType::COMMA, ",");
    assertToken(tokens[6], TokenType::END_OF_FILE, "");
    
    std::cout << "Delimiters test passed!" << std::endl;
}

// 测试空白字符处理
void testWhitespace() {
    std::cout << "Testing whitespace handling..." << std::endl;
    
    Lexer lexer("  int   \t\n  variable  \n\n  123  ");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 4); // 3 tokens + EOF
    assertToken(tokens[0], TokenType::INT, "int", 1);        // 第1行
    assertToken(tokens[1], TokenType::IDENTIFIER, "variable", 2);  // 第2行
    assertToken(tokens[2], TokenType::NUMBER, "123", 4);     // 第4行 (因为有两个连续的\n)
    assertToken(tokens[3], TokenType::END_OF_FILE, "", 4);   // EOF在第4行
    
    std::cout << "Whitespace handling test passed!" << std::endl;
}

// 测试行列位置跟踪
void testLineColumnTracking() {
    std::cout << "Testing line and column tracking..." << std::endl;
    
    Lexer lexer("int\nvar = 123;\nreturn var;");
    std::vector<Token> tokens = lexer.tokenize();
    
    // 检查第一行
    assertToken(tokens[0], TokenType::INT, "int", 1, 1);
    
    // 检查第二行
    assertToken(tokens[1], TokenType::IDENTIFIER, "var", 2, 1);
    assertToken(tokens[2], TokenType::ASSIGN, "=", 2, 5);
    assertToken(tokens[3], TokenType::NUMBER, "123", 2, 7);
    assertToken(tokens[4], TokenType::SEMICOLON, ";", 2, 10);
    
    // 检查第三行
    assertToken(tokens[5], TokenType::RETURN, "return", 3, 1);
    assertToken(tokens[6], TokenType::IDENTIFIER, "var", 3, 8);
    assertToken(tokens[7], TokenType::SEMICOLON, ";", 3, 11);
    
    std::cout << "Line and column tracking test passed!" << std::endl;
}

// 测试复杂表达式
void testComplexExpression() {
    std::cout << "Testing complex expression..." << std::endl;
    
    Lexer lexer("if (x == 10) { return x * 2; }");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 14); // 13 tokens + EOF
    assertToken(tokens[0], TokenType::IF, "if");
    assertToken(tokens[1], TokenType::LPAREN, "(");
    assertToken(tokens[2], TokenType::IDENTIFIER, "x");
    assertToken(tokens[3], TokenType::EQ, "==");
    assertToken(tokens[4], TokenType::NUMBER, "10");
    assertToken(tokens[5], TokenType::RPAREN, ")");
    assertToken(tokens[6], TokenType::LBRACE, "{");
    assertToken(tokens[7], TokenType::RETURN, "return");
    assertToken(tokens[8], TokenType::IDENTIFIER, "x");
    assertToken(tokens[9], TokenType::STAR, "*");
    assertToken(tokens[10], TokenType::NUMBER, "2");
    assertToken(tokens[11], TokenType::SEMICOLON, ";");
    assertToken(tokens[12], TokenType::RBRACE, "}");
    assertToken(tokens[13], TokenType::END_OF_FILE, "");
    
    std::cout << "Complex expression test passed!" << std::endl;
}

// 测试无效字符
void testInvalidCharacters() {
    std::cout << "Testing invalid characters..." << std::endl;
    
    Lexer lexer("@ # $");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 4); // 3 invalid tokens + EOF
    assertToken(tokens[0], TokenType::INVALID, "@");
    assertToken(tokens[1], TokenType::INVALID, "#");
    assertToken(tokens[2], TokenType::INVALID, "$");
    assertToken(tokens[3], TokenType::END_OF_FILE, "");
    
    std::cout << "Invalid characters test passed!" << std::endl;
}

// 测试空输入
void testEmptyInput() {
    std::cout << "Testing empty input..." << std::endl;
    
    Lexer lexer("");
    std::vector<Token> tokens = lexer.tokenize();
    
    assert(tokens.size() == 1); // Only EOF
    assertToken(tokens[0], TokenType::END_OF_FILE, "");
    
    std::cout << "Empty input test passed!" << std::endl;
}

// 测试单个token的nextToken方法
void testNextTokenMethod() {
    std::cout << "Testing nextToken method..." << std::endl;
    
    Lexer lexer("int x = 5;");
    
    Token t1 = lexer.nextToken();
    assertToken(t1, TokenType::INT, "int");
    
    Token t2 = lexer.nextToken();
    assertToken(t2, TokenType::IDENTIFIER, "x");
    
    Token t3 = lexer.nextToken();
    assertToken(t3, TokenType::ASSIGN, "=");
    
    Token t4 = lexer.nextToken();
    assertToken(t4, TokenType::NUMBER, "5");
    
    Token t5 = lexer.nextToken();
    assertToken(t5, TokenType::SEMICOLON, ";");
    
    Token t6 = lexer.nextToken();
    assertToken(t6, TokenType::END_OF_FILE, "");
    
    // 继续调用应该仍然返回EOF
    Token t7 = lexer.nextToken();
    assertToken(t7, TokenType::END_OF_FILE, "");
    
    std::cout << "nextToken method test passed!" << std::endl;
}

// 测试函数定义示例
void testFunctionExample() {
    std::cout << "Testing function example..." << std::endl;
    
    std::string code = R"(int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
})";
    
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    
    // 验证一些关键token
    assert(tokens.size() > 20); // 应该有很多token
    
    // 检查函数签名部分
    int idx = 0;
    assertToken(tokens[idx++], TokenType::INT, "int", 1);  // 现在在第1行
    assertToken(tokens[idx++], TokenType::IDENTIFIER, "factorial", 1);
    assertToken(tokens[idx++], TokenType::LPAREN, "(", 1);
    assertToken(tokens[idx++], TokenType::INT, "int", 1);
    assertToken(tokens[idx++], TokenType::IDENTIFIER, "n", 1);
    assertToken(tokens[idx++], TokenType::RPAREN, ")", 1);
    assertToken(tokens[idx++], TokenType::LBRACE, "{", 1);
    
    std::cout << "Function example test passed!" << std::endl;
}

int main() {
    std::cout << "Starting Lexer Unit Tests..." << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        testKeywords();
        testIdentifiers();
        testNumbers();
        testArithmeticOperators();
        testComparisonOperators();
        testAssignmentOperator();
        testDelimiters();
        testWhitespace();
        testLineColumnTracking();
        testComplexExpression();
        testInvalidCharacters();
        testEmptyInput();
        testNextTokenMethod();
        testFunctionExample();
        
        std::cout << "=============================" << std::endl;
        std::cout << "All tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
