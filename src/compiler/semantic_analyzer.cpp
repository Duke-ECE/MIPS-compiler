#include "compiler/semantic_analyzer.hpp"
#include <sstream>

// ===== 构造函数 =====
SemanticAnalyzer::SemanticAnalyzer() {
    initBuiltinFunctions();
}

// ===== 初始化内建函数 =====
void SemanticAnalyzer::initBuiltinFunctions() {
    // input() 返回 int，无参数
    // output(int) 返回 void（在我们的系统中用 "void" 表示）
    // 注意：这里我们将内建函数注册到全局作用域
    // 在 visitProgram 中会 enterScope，所以这里不需要
}

// ===== Helper：类型检查 =====
void SemanticAnalyzer::ensureType(const std::string &type,
                                  const std::string &expected,
                                  const std::string &errMsg)
{
    if (type != expected) {
        throw SemanticError(errMsg + " (expected: " + expected +
                            ", got: " + type + ")");
    }
}

// ===== 入口 =====
void SemanticAnalyzer::analyze(ASTProgram *program) {
    visitProgram(program);
}

// ===== Dispatch =====
void SemanticAnalyzer::visit(ASTNode *node) {
    switch (node->kind) {
        case ASTNodeKind::Program:   visitProgram(static_cast<ASTProgram*>(node)); break;
        case ASTNodeKind::Function:  visitFunction(static_cast<ASTFunction*>(node)); break;
        case ASTNodeKind::Block:     visitBlock(static_cast<ASTBlock*>(node)); break;

        case ASTNodeKind::VarDecl:   visitVarDecl(static_cast<ASTVarDecl*>(node)); break;
        case ASTNodeKind::If:        visitIf(static_cast<ASTIf*>(node)); break;
        case ASTNodeKind::While:     visitWhile(static_cast<ASTWhile*>(node)); break;
        case ASTNodeKind::Return:    visitReturn(static_cast<ASTReturn*>(node)); break;
        case ASTNodeKind::ExprStmt:  visitExprStmt(static_cast<ASTExprStmt*>(node)); break;

        case ASTNodeKind::BinaryExpr:
        case ASTNodeKind::UnaryExpr:
        case ASTNodeKind::IdentifierExpr:
        case ASTNodeKind::NumberExpr:
        case ASTNodeKind::CallExpr:
        case ASTNodeKind::AssignExpr:
            visitExpr(static_cast<ASTExpression*>(node));
            break;

        default:
            throw SemanticError("Unknown AST node kind");
    }
}

// ===== Program =====
void SemanticAnalyzer::visitProgram(ASTProgram *node) {
    symbols.enterScope(); // global scope

    // 注册内建函数
    symbols.declare("input", "int");   // int input()
    symbols.declare("output", "void"); // void output(int)

    for (auto &func : node->functions) {
        // 函数名放入 global symbol table（类型 = returnType）
        if (!symbols.declare(func->name, func->returnType)) {
            throw SemanticError("Redefinition of function: " + func->name);
        }
    }

    // 逐个函数继续做语义分析
    for (auto &func : node->functions) {
        visitFunction(func.get());
    }

    symbols.exitScope();
}

// ===== Function =====
void SemanticAnalyzer::visitFunction(ASTFunction *node) {
    symbols.enterScope(); // 函数内部 scope

    // 参数注册到符号表
    for (size_t i = 0; i < node->paramNames.size(); ++i) {
        if (!symbols.declare(node->paramNames[i], node->paramTypes[i])) {
            throw SemanticError("Duplicate parameter name: " + node->paramNames[i]);
        }
    }

    // 函数体检查
    visit(node->body.get());

    symbols.exitScope();
}

// ===== Block =====
void SemanticAnalyzer::visitBlock(ASTBlock *node) {
    symbols.enterScope();

    for (auto &stmt : node->statements) {
        visit(stmt.get());
    }

    symbols.exitScope();
}

// ===== Variable Declaration =====
void SemanticAnalyzer::visitVarDecl(ASTVarDecl *node) {
    std::string name = node->name;
    std::string type = node->typeName; // currently "int"

    if (!symbols.declare(name, type)) {
        throw SemanticError("Variable redeclared: " + name);
    }

    if (node->initExpr) {
        std::string t = visitExpr(node->initExpr.get());
        ensureType(t, type, "Type mismatch in initialization of variable " + name);
    }
}

// ===== If =====
void SemanticAnalyzer::visitIf(ASTIf *node) {
    std::string condType = visitExpr(node->condition.get());
    ensureType(condType, "int", "Condition of if must be int");

    visit(node->thenBranch.get());
    if (node->elseBranch) {
        visit(node->elseBranch.get());
    }
}

// ===== While =====
void SemanticAnalyzer::visitWhile(ASTWhile *node) {
    std::string condType = visitExpr(node->condition.get());
    ensureType(condType, "int", "Condition of while must be int");

    visit(node->body.get());
}

// ===== Return =====
void SemanticAnalyzer::visitReturn(ASTReturn *node) {
    if (node->value) {
        std::string t = visitExpr(node->value.get());
        ensureType(t, "int", "Return type mismatch");
    }
}

// ===== Expr Statement =====
void SemanticAnalyzer::visitExprStmt(ASTExprStmt *node) {
    visitExpr(node->expr.get());
}

// =============================================================
// =============== Expression Visitors ==========================
// =============================================================
std::string SemanticAnalyzer::visitExpr(ASTExpression *expr) {
    switch (expr->kind) {
        case ASTNodeKind::BinaryExpr:
            return visitBinaryExpr(static_cast<ASTBinaryExpr*>(expr));
        case ASTNodeKind::UnaryExpr:
            return visitUnaryExpr(static_cast<ASTUnaryExpr*>(expr));
        case ASTNodeKind::IdentifierExpr:
            return visitIdentifier(static_cast<ASTIdentifierExpr*>(expr));
        case ASTNodeKind::NumberExpr:
            return visitNumber(static_cast<ASTNumberExpr*>(expr));
        case ASTNodeKind::CallExpr:
            return visitCall(static_cast<ASTCallExpr*>(expr));
        case ASTNodeKind::AssignExpr:
            return visitAssign(static_cast<ASTAssignExpr*>(expr));
        default:
            throw SemanticError("Unknown expression node");
    }
}

// ===== Binary =====
std::string SemanticAnalyzer::visitBinaryExpr(ASTBinaryExpr *node) {
    std::string lt = visitExpr(node->left.get());
    std::string rt = visitExpr(node->right.get());

    if (lt != "int" || rt != "int") {
        throw SemanticError("Binary operator requires int operands");
    }
    return "int";  // arithmetic result is int
}

// ===== Unary =====
std::string SemanticAnalyzer::visitUnaryExpr(ASTUnaryExpr *node) {
    std::string t = visitExpr(node->expr.get());
    ensureType(t, "int", "Unary operator requires int operand");
    return "int";
}

// ===== Identifier =====
std::string SemanticAnalyzer::visitIdentifier(ASTIdentifierExpr *node) {
    std::string t = symbols.lookup(node->name);
    if (t.empty()) {
        throw SemanticError("Use of undeclared variable: " + node->name);
    }
    return t;
}

// ===== Number =====
std::string SemanticAnalyzer::visitNumber(ASTNumberExpr *node) {
    return "int";
}

// ===== Call =====
std::string SemanticAnalyzer::visitCall(ASTCallExpr *node) {
    // 特殊处理内建函数 input 和 output
    if (node->callee == "input") {
        // input() 无参数
        if (!node->arguments.empty()) {
            throw SemanticError("input() takes no arguments");
        }
        return "int";
    }
    
    if (node->callee == "output") {
        // output(int) 接受一个 int 参数
        if (node->arguments.size() != 1) {
            throw SemanticError("output() takes exactly one argument");
        }
        std::string argType = visitExpr(node->arguments[0].get());
        ensureType(argType, "int", "output() argument must be int");
        return "void";
    }
    
    // 查找函数
    std::string ft = symbols.lookup(node->callee);
    if (ft.empty()) {
        throw SemanticError("Call to undeclared function: " + node->callee);
    }

    // 参数类型检查（当前只检查每个参数都是 int）
    for (auto &arg : node->arguments) {
        std::string t = visitExpr(arg.get());
        ensureType(t, "int", "Function argument must be int");
    }

    return ft; // 函数返回类型
}

// ===== Assignment =====
std::string SemanticAnalyzer::visitAssign(ASTAssignExpr *node) {
    std::string varType = symbols.lookup(node->name);
    if (varType.empty()) {
        throw SemanticError("Assignment to undeclared variable: " + node->name);
    }

    std::string valueType = visitExpr(node->value.get());
    ensureType(valueType, varType, "Assignment type mismatch for " + node->name);

    return varType;
}
