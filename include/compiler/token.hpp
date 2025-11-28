#pragma once
#include <string>

enum class TokenType {
    // Keywords
    INT, RETURN, IF, ELSE, WHILE,

    // Identifiers & literals
    IDENTIFIER,
    NUMBER,

    // Operators
    PLUS, MINUS, STAR, SLASH,
    ASSIGN,        // =
    EQ,            // ==
    NEQ,           // !=
    LT, GT, LE, GE,

    // Delimiters
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    SEMICOLON,
    COMMA,

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
