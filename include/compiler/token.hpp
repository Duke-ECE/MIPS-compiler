/*
token.hpp: MIPS 编译器的记号定义
作用： 定义词法分析器生成的各种记号类型及其结构。
*/
#pragma once
#include <string>

enum class TokenType {
    // Keywords
    INT, RETURN, IF, ELSE, WHILE,

    // Identifiers & literals
    IDENTIFIER,// 变量名、函数名等
    NUMBER,// 数字字面量

    // Operators
    PLUS, MINUS, STAR, SLASH,// +, -, *, /
    ASSIGN,        // =
    EQ,            // ==
    NEQ,           // !=
    LT, GT, LE, GE, // <, >, <=, >=

    // Delimiters
    LPAREN, RPAREN,// (, )
    LBRACE, RBRACE,// {, }
    SEMICOLON,// ;
    COMMA,// ,

    // End of file
    END_OF_FILE,

    // Error token
    INVALID
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;

    Token(TokenType t, const std::string &txt, int ln, int col)
        : type(t), text(txt), line(ln), column(col) {}
};
