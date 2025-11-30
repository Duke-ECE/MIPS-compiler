/*
semantic_analyzer.hpp: 语义分析器的声明
作用：对 AST 进行语义检查，确保变量声明、类型匹配等符合语言规范
*/
#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "ast.hpp"
#include "symbol_table.hpp"
#include <string>
#include <stdexcept>

class SemanticError : public std::runtime_error {
public:
    explicit SemanticError(const std::string &msg)
        : std::runtime_error(msg) {}
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    // 入口：语义检查整个程序
    void analyze(ASTProgram *program);

private:
    SymbolTable symbols;

    // 初始化内建函数
    void initBuiltinFunctions();

private:
    // dispatch：不同节点的 visit 方法
    void visit(ASTNode *node);

    // Program, Function, Block
    void visitProgram(ASTProgram *node);
    void visitFunction(ASTFunction *node);
    void visitBlock(ASTBlock *node);

    // Statement
    void visitVarDecl(ASTVarDecl *node);
    void visitIf(ASTIf *node);
    void visitWhile(ASTWhile *node);
    void visitReturn(ASTReturn *node);
    void visitExprStmt(ASTExprStmt *node);

    // Expression
    std::string visitExpr(ASTExpression *expr);
    std::string visitBinaryExpr(ASTBinaryExpr *node);
    std::string visitUnaryExpr(ASTUnaryExpr *node);
    std::string visitIdentifier(ASTIdentifierExpr *node);
    std::string visitNumber(ASTNumberExpr *node);
    std::string visitCall(ASTCallExpr *node);
    std::string visitAssign(ASTAssignExpr *node);

    // helper
    void ensureType(const std::string &type, const std::string &expected,
                    const std::string &errMsg);
};

#endif // SEMANTIC_ANALYZER_HPP
