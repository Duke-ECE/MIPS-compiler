#include "compiler/lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string &input)
    : input(input), pos(0), line(1), col(1) {}

bool Lexer::eof() const {
    return pos >= input.size();
}

char Lexer::peek() const {
    if (eof()) return '\0';
    return input[pos];
}

char Lexer::get() {
    char c = peek();
    pos++;
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (!eof() && std::isspace(peek())) {
        get();
    }
}

Token Lexer::identifierOrKeyword() {
    int startCol = col;
    std::string text;
    while (!eof() && (std::isalnum(peek()) || peek() == '_')) {
        text.push_back(get());
    }

    if (text == "int")    return Token(TokenType::INT, text, line, startCol);
    if (text == "return") return Token(TokenType::RETURN, text, line, startCol);
    if (text == "if")     return Token(TokenType::IF, text, line, startCol);
    if (text == "else")   return Token(TokenType::ELSE, text, line, startCol);
    if (text == "while")  return Token(TokenType::WHILE, text, line, startCol);

    return Token(TokenType::IDENTIFIER, text, line, startCol);
}

Token Lexer::number() {
    int startCol = col;
    std::string text;
    
    // 检查是否为十六进制（0x 或 0X）
    if (peek() == '0' && !eof()) {
        text.push_back(get());
        if (!eof() && (peek() == 'x' || peek() == 'X')) {
            text.push_back(get());
            // 读取十六进制数字
            while (!eof() && std::isxdigit(peek())) {
                text.push_back(get());
            }
            return Token(TokenType::NUMBER, text, line, startCol);
        }
    }
    
    // 普通十进制数字
    while (!eof() && std::isdigit(peek())) {
        text.push_back(get());
    }
    return Token(TokenType::NUMBER, text, line, startCol);
}

Token Lexer::symbol() {
    int startCol = col;
    char c = get();

    // Multi-char operators
    if (c == '=' && peek() == '=') { get(); return Token(TokenType::EQ, "==", line, startCol); }
    if (c == '!' && peek() == '=') { get(); return Token(TokenType::NEQ, "!=", line, startCol); }
    if (c == '<' && peek() == '=') { get(); return Token(TokenType::LE, "<=", line, startCol); }
    if (c == '>' && peek() == '=') { get(); return Token(TokenType::GE, ">=", line, startCol); }

    // Single-char operators
    switch (c) {
        case '+': return Token(TokenType::PLUS, "+", line, startCol);
        case '-': return Token(TokenType::MINUS, "-", line, startCol);
        case '*': return Token(TokenType::STAR, "*", line, startCol);
        case '/': return Token(TokenType::SLASH, "/", line, startCol);
        case '=': return Token(TokenType::ASSIGN, "=", line, startCol);
        case '<': return Token(TokenType::LT, "<", line, startCol);
        case '>': return Token(TokenType::GT, ">", line, startCol);

        case '(': return Token(TokenType::LPAREN, "(", line, startCol);
        case ')': return Token(TokenType::RPAREN, ")", line, startCol);
        case '{': return Token(TokenType::LBRACE, "{", line, startCol);
        case '}': return Token(TokenType::RBRACE, "}", line, startCol);
        case ';': return Token(TokenType::SEMICOLON, ";", line, startCol);
        case ',': return Token(TokenType::COMMA, ",", line, startCol);
    }

    return Token(TokenType::INVALID, std::string(1, c), line, startCol);
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (eof())
        return Token(TokenType::END_OF_FILE, "", line, col);

    char c = peek();

    if (std::isalpha(c) || c == '_') return identifierOrKeyword();
    if (std::isdigit(c))             return number();

    return symbol();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        if (t.type == TokenType::END_OF_FILE)
            break;
    }
    return tokens;
}

