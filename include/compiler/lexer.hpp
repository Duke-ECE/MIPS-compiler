/*
Lexar: MIPS 编译器的词法分析器
作用： 将源代码转换为一系列记号（tokens），供后续的语法分析使用。
*/

#pragma once
#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
public:
    explicit Lexer(const std::string &input);

    Token nextToken();     // 获取下一个 token
    bool eof() const;      // 判断是否结束
    std::vector<Token> tokenize();  // 一次性返回全部 token

private:
    char peek() const;
    char get();
    void skipWhitespace();
    Token identifierOrKeyword();
    Token number();
    Token symbol();

private:
    std::string input;
    size_t pos;
    int line, col;
};


