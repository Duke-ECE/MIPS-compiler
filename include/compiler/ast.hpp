/*
ast.hpp: MIPS 编译器的抽象语法树定义
作用： 定义编译器前端生成的抽象语法树（AST）节点类型及其结构。
*/
#ifndef AST_HPP
#define AST_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <ostream>

// -------------------- 节点类型枚举 --------------------

enum class ASTNodeKind {
    Program,
    Function,
    Block,

    // 语句
    VarDecl,
    If,
    While,
    Return,
    ExprStmt,

    // 表达式
    BinaryExpr,
    UnaryExpr,
    IdentifierExpr,
    NumberExpr,
    CallExpr,
    AssignExpr
};

// 二元运算符
enum class ASTBinaryOpKind {
    Add,            // +
    Sub,            // -
    Mul,            // *
    Div,            // /

    Less,           // <
    LessEqual,      // <=
    Greater,        // >
    GreaterEqual,   // >=

    Equal,          // ==
    NotEqual        // !=
};

// 一元运算符
enum class ASTUnaryOpKind {
    Neg,    // -x
    Not     // !x
};

// -------------------- AST 基类 --------------------

class ASTNode {
public:
    ASTNodeKind kind;

    explicit ASTNode(ASTNodeKind k) : kind(k) {}
    virtual ~ASTNode() = default;
};

// “语句”基类
class ASTStatement : public ASTNode {
public:
    explicit ASTStatement(ASTNodeKind k) : ASTNode(k) {}
    ~ASTStatement() override = default;
};

// “表达式”基类
class ASTExpression : public ASTNode {
public:
    explicit ASTExpression(ASTNodeKind k) : ASTNode(k) {}
    ~ASTExpression() override = default;
};

// -------------------- 顶层 Program / Function / Block --------------------

// program → (function_definition)*
class ASTFunction;
class ASTProgram : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTFunction>> functions;

    ASTProgram();
    ~ASTProgram() override;
};

// int foo(...) { ... }
class ASTBlock;
class ASTFunction : public ASTNode {
public:
    std::string returnType;               // 当前只用 "int"
    std::string name;

    std::vector<std::string> paramTypes;  // 一般都是 "int"
    std::vector<std::string> paramNames;

    std::unique_ptr<ASTBlock> body;       // 函数体

    ASTFunction();
    ~ASTFunction() override;
};

// { stmt1; stmt2; ... }
class ASTBlock : public ASTStatement {
public:
    std::vector<std::unique_ptr<ASTStatement>> statements;

    ASTBlock();
    ~ASTBlock() override;
};

// -------------------- 语句节点 --------------------

// int x = expr;
class ASTVarDecl : public ASTStatement {
public:
    std::string typeName;                        // int
    std::string name;
    std::unique_ptr<ASTExpression> initExpr;     // 可能为 nullptr

    ASTVarDecl();
    ~ASTVarDecl() override;
};

// if (cond) thenBranch else elseBranch;
class ASTIf : public ASTStatement {
public:
    std::unique_ptr<ASTExpression> condition;
    std::unique_ptr<ASTStatement> thenBranch;
    std::unique_ptr<ASTStatement> elseBranch;    // 可能为 nullptr

    ASTIf();
    ~ASTIf() override;
};

// while (cond) body;
class ASTWhile : public ASTStatement {
public:
    std::unique_ptr<ASTExpression> condition;
    std::unique_ptr<ASTStatement> body;

    ASTWhile();
    ~ASTWhile() override;
};

// return expr;
class ASTReturn : public ASTStatement {
public:
    std::unique_ptr<ASTExpression> value;        // 可能为 nullptr（return;）

    ASTReturn();
    ~ASTReturn() override;
};

// expression;
class ASTExprStmt : public ASTStatement {
public:
    std::unique_ptr<ASTExpression> expr;

    ASTExprStmt();
    ~ASTExprStmt() override;
};

// -------------------- 表达式节点 --------------------

// left (op) right
class ASTBinaryExpr : public ASTExpression {
public:
    ASTBinaryOpKind op;
    std::unique_ptr<ASTExpression> left;
    std::unique_ptr<ASTExpression> right;

    ASTBinaryExpr();
    ~ASTBinaryExpr() override;
};

// op expr
class ASTUnaryExpr : public ASTExpression {
public:
    ASTUnaryOpKind op;
    std::unique_ptr<ASTExpression> expr;

    ASTUnaryExpr();
    ~ASTUnaryExpr() override;
};

// 标识符：变量名
class ASTIdentifierExpr : public ASTExpression {
public:
    std::string name;

    ASTIdentifierExpr();
    ~ASTIdentifierExpr() override;
};

// 整数常量
class ASTNumberExpr : public ASTExpression {
public:
    std::int32_t value;   // 够用，后端映射到 MIPS 立即数

    ASTNumberExpr();
    ~ASTNumberExpr() override;
};

// 函数调用：foo(a, b+1)
class ASTCallExpr : public ASTExpression {
public:
    std::string callee;
    std::vector<std::unique_ptr<ASTExpression>> arguments;

    ASTCallExpr();
    ~ASTCallExpr() override;
};

// 赋值表达式：a = expr
class ASTAssignExpr : public ASTExpression {
public:
    std::string name;
    std::unique_ptr<ASTExpression> value;

    ASTAssignExpr();
    ~ASTAssignExpr() override;
};

// -------------------- 调试输出（可选） --------------------
// 方便单元测试时打印 AST 结构
void dumpAST(const ASTProgram &program, std::ostream &os, int indent = 0);

#endif // AST_HPP