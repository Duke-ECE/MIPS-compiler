/**
 * @file ir_builder.hpp
 * @brief Intermediate Representation (IR) Builder from AST
 * 
 * This file defines the IRBuilder class that converts an Abstract Syntax Tree (AST)
 * into an Intermediate Representation (IR) suitable for further processing and code generation.
 * 
 * IRBuilder class:
 * - generateIR: Main function to convert AST to IRProgram.
 * - genStmt/genExpr: Helper functions to generate IR for statements and expressions.
 * - Various private methods to handle specific AST node types.
 * 
 * Notes:
 * - This IRBuilder assumes a simple three-address code representation.
 */
#ifndef IR_BUILDER_HPP
#define IR_BUILDER_HPP

#include "ast.hpp"
#include "ir.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class IRBuilder {
public:
    IRBuilder();

    // ASTNode* → IRProgram
    IRProgram generateIR(const ASTNode *root);

private:
    IRProgram program;

    // 临时变量计数（t0, t1, t2...）
    int tempCounter;
    
    // MIPS调用约定支持
    int frameSize;                                    // 当前函数栈帧大小
    int localVarOffset;                              // 当前局部变量偏移
    std::unordered_map<std::string, int> localVars;  // 变量名 -> 栈偏移映射
    bool hasExplicitReturn;                          // 当前函数是否有显式return语句

    // 生成一个新的临时变量名
    std::string newTemp();
    
    // MIPS栈帧管理
    int allocateLocalVar(const std::string &varName);   // 为局部变量分配栈空间
    void calculateFrameSize(const ASTFunction *func);   // 计算函数栈帧大小

    // 生成语句和表达式的 IR
    void genStmt(const ASTStatement *stmt);
    std::string genExpr(const ASTExpression *expr);

    // 对各种节点的处理函数
    void genProgram(const ASTProgram *node);
    void genFunction(const ASTFunction *node);
    void genBlock(const ASTBlock *node);
    void genVarDecl(const ASTVarDecl *node);
    void genIf(const ASTIf *node);
    void genWhile(const ASTWhile *node);
    void genReturn(const ASTReturn *node);
    void genExprStmt(const ASTExprStmt *node);

    // 表达式节点
    std::string genBinary(const ASTBinaryExpr *node);
    std::string genUnary(const ASTUnaryExpr *node);
    std::string genIdentifier(const ASTIdentifierExpr *node);
    std::string genNumber(const ASTNumberExpr *node);
    std::string genCall(const ASTCallExpr *node);
    std::string genAssign(const ASTAssignExpr *node);
    std::string genStore(const ASTStoreExpr *node);
};

#endif // IR_BUILDER_HPP
