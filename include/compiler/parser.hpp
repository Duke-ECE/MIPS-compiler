/*
parser.hpp: MIPS 编译器的语法分析器定义
作用： 定义将词法分析器生成的记号序列解析为抽象语法树（AST）的语法分析器类。
*/
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

#include "token.hpp"
#include "ast.hpp"

// 简单的语法错误异常类型
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &msg)
        : std::runtime_error(msg) {}
};

class Parser {
public:
    explicit Parser(const std::vector<Token> &tokens);

    // program → (function_definition)*
    std::unique_ptr<ASTProgram> parseProgram();

private:
    // === 基础工具函数 ===
    const Token &peek(int offset = 0) const;     // 查看当前/之后的 token，不前进
    const Token &previous() const;              // 最近消费的 token
    bool isAtEnd() const;                       // 是否到达 EOF
    const Token &advance();                     // 消费一个 token
    bool check(TokenType type) const;           // 当前 token 是否为某种类型
    bool match(TokenType type);                 // 如果当前是 type 则消费并返回 true
    bool match(std::initializer_list<TokenType> types); // 多选其一
    const Token &consume(TokenType type, const std::string &errMsg);

    // === 顶层结构 ===
    // function_definition → "int" IDENT "(" parameters? ")" block
    std::unique_ptr<ASTFunction> parseFunction();

    // parameters → "int" IDENT ("," "int" IDENT)* | ε
    void parseParameterList(ASTFunction &func);

    // block → "{" statement* "}"
    std::unique_ptr<ASTBlock> parseBlock();

    // statement*（用于 block 内扫描）
    std::unique_ptr<ASTStatement> parseStatement();

    // === 各种语句 ===
    // declaration → "int" IDENT ("=" expression)? ";"
    std::unique_ptr<ASTStatement> parseDeclarationStatement();

    // if_statement → "if" "(" expression ")" statement ( "else" statement )?
    std::unique_ptr<ASTStatement> parseIfStatement();

    // while_statement → "while" "(" expression ")" statement
    std::unique_ptr<ASTStatement> parseWhileStatement();

    // return_statement → "return" expression? ";"
    std::unique_ptr<ASTStatement> parseReturnStatement();

    // expression_statement → expression ";"
    std::unique_ptr<ASTStatement> parseExpressionStatement();

    // === 表达式（带优先级）===
    // expression → assignment
    std::unique_ptr<ASTExpression> parseExpression();

    // assignment → IDENT "=" assignment | equality
    std::unique_ptr<ASTExpression> parseAssignment();

    // equality → comparison ( ("==" | "!=") comparison )*
    std::unique_ptr<ASTExpression> parseEquality();

    // comparison → additive ( ("<" | "<=" | ">" | ">=") additive )*
    std::unique_ptr<ASTExpression> parseComparison();

    // additive → multiplicative ( ("+" | "-") multiplicative )*
    std::unique_ptr<ASTExpression> parseAdditive();

    // multiplicative → unary ( ("*" | "/") unary )*
    std::unique_ptr<ASTExpression> parseMultiplicative();

    // unary → ("!" | "-") unary | primary
    std::unique_ptr<ASTExpression> parseUnary();

    // primary → NUMBER | IDENT | "(" expression ")" | function_call
    std::unique_ptr<ASTExpression> parsePrimary();

    // function_call → IDENT "(" argument_list? ")"
    std::unique_ptr<ASTExpression> finishCall(const std::string &funcName);

    // argument_list → expression ("," expression)*
    void parseArgumentList(std::vector<std::unique_ptr<ASTExpression>> &args);

private:
    const std::vector<Token> &tokens;
    size_t current;
};

#endif // PARSER_HPP