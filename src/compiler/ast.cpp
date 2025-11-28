#include "compiler/ast.hpp"

// 一个简单缩进工具
static void printIndent(std::ostream &os, int indent) {
    for (int i = 0; i < indent; ++i) {
        os << "  ";
    }
}

// -------------------- 构造 / 析构实现 --------------------

// Program
ASTProgram::ASTProgram()
    : ASTNode(ASTNodeKind::Program) {}

ASTProgram::~ASTProgram() = default;

// Function
ASTFunction::ASTFunction()
    : ASTNode(ASTNodeKind::Function) {}

ASTFunction::~ASTFunction() = default;

// Block
ASTBlock::ASTBlock()
    : ASTStatement(ASTNodeKind::Block) {}

ASTBlock::~ASTBlock() = default;

// VarDecl
ASTVarDecl::ASTVarDecl()
    : ASTStatement(ASTNodeKind::VarDecl) {}

ASTVarDecl::~ASTVarDecl() = default;

// If
ASTIf::ASTIf()
    : ASTStatement(ASTNodeKind::If) {}

ASTIf::~ASTIf() = default;

// While
ASTWhile::ASTWhile()
    : ASTStatement(ASTNodeKind::While) {}

ASTWhile::~ASTWhile() = default;

// Return
ASTReturn::ASTReturn()
    : ASTStatement(ASTNodeKind::Return) {}

ASTReturn::~ASTReturn() = default;

// ExprStmt
ASTExprStmt::ASTExprStmt()
    : ASTStatement(ASTNodeKind::ExprStmt) {}

ASTExprStmt::~ASTExprStmt() = default;

// BinaryExpr
ASTBinaryExpr::ASTBinaryExpr()
    : ASTExpression(ASTNodeKind::BinaryExpr),
      op(ASTBinaryOpKind::Add) {}

ASTBinaryExpr::~ASTBinaryExpr() = default;

// UnaryExpr
ASTUnaryExpr::ASTUnaryExpr()
    : ASTExpression(ASTNodeKind::UnaryExpr),
      op(ASTUnaryOpKind::Neg) {}

ASTUnaryExpr::~ASTUnaryExpr() = default;

// IdentifierExpr
ASTIdentifierExpr::ASTIdentifierExpr()
    : ASTExpression(ASTNodeKind::IdentifierExpr) {}

ASTIdentifierExpr::~ASTIdentifierExpr() = default;

// NumberExpr
ASTNumberExpr::ASTNumberExpr()
    : ASTExpression(ASTNodeKind::NumberExpr),
      value(0) {}

ASTNumberExpr::~ASTNumberExpr() = default;

// CallExpr
ASTCallExpr::ASTCallExpr()
    : ASTExpression(ASTNodeKind::CallExpr) {}

ASTCallExpr::~ASTCallExpr() = default;

// AssignExpr
ASTAssignExpr::ASTAssignExpr()
    : ASTExpression(ASTNodeKind::AssignExpr) {}

ASTAssignExpr::~ASTAssignExpr() = default;

// -------------------- AST dump（调试用） --------------------

static void dumpExpr(const ASTExpression *expr, std::ostream &os, int indent);
static void dumpStmt(const ASTStatement *stmt, std::ostream &os, int indent);

static const char *binaryOpToString(ASTBinaryOpKind op) {
    switch (op) {
    case ASTBinaryOpKind::Add:          return "+";
    case ASTBinaryOpKind::Sub:          return "-";
    case ASTBinaryOpKind::Mul:          return "*";
    case ASTBinaryOpKind::Div:          return "/";
    case ASTBinaryOpKind::Less:         return "<";
    case ASTBinaryOpKind::LessEqual:    return "<=";
    case ASTBinaryOpKind::Greater:      return ">";
    case ASTBinaryOpKind::GreaterEqual: return ">=";
    case ASTBinaryOpKind::Equal:        return "==";
    case ASTBinaryOpKind::NotEqual:     return "!=";
    }
    return "?";
}

static const char *unaryOpToString(ASTUnaryOpKind op) {
    switch (op) {
    case ASTUnaryOpKind::Neg: return "-";
    case ASTUnaryOpKind::Not: return "!";
    }
    return "?";
}

void dumpAST(const ASTProgram &program, std::ostream &os, int indent) {
    printIndent(os, indent);
    os << "Program\n";

    for (const auto &funcPtr : program.functions) {
        const ASTFunction *fn = funcPtr.get();
        printIndent(os, indent + 1);
        os << "Function " << fn->returnType << " " << fn->name << "(";

        for (size_t i = 0; i < fn->paramNames.size(); ++i) {
            if (i > 0) os << ", ";
            os << fn->paramTypes[i] << " " << fn->paramNames[i];
        }
        os << ")\n";

        if (fn->body) {
            dumpStmt(fn->body.get(), os, indent + 2);
        }
    }
}

// --- dump 语句 ---

static void dumpStmt(const ASTStatement *stmt, std::ostream &os, int indent) {
    switch (stmt->kind) {
    case ASTNodeKind::Block: {
        auto block = static_cast<const ASTBlock *>(stmt);
        printIndent(os, indent);
        os << "Block\n";
        for (const auto &s : block->statements) {
            dumpStmt(s.get(), os, indent + 1);
        }
        break;
    }
    case ASTNodeKind::VarDecl: {
        auto decl = static_cast<const ASTVarDecl *>(stmt);
        printIndent(os, indent);
        os << "VarDecl " << decl->typeName << " " << decl->name << "\n";
        if (decl->initExpr) {
            printIndent(os, indent + 1);
            os << "Init:\n";
            dumpExpr(decl->initExpr.get(), os, indent + 2);
        }
        break;
    }
    case ASTNodeKind::If: {
        auto ifs = static_cast<const ASTIf *>(stmt);
        printIndent(os, indent);
        os << "If\n";

        printIndent(os, indent + 1);
        os << "Cond:\n";
        dumpExpr(ifs->condition.get(), os, indent + 2);

        printIndent(os, indent + 1);
        os << "Then:\n";
        dumpStmt(ifs->thenBranch.get(), os, indent + 2);

        if (ifs->elseBranch) {
            printIndent(os, indent + 1);
            os << "Else:\n";
            dumpStmt(ifs->elseBranch.get(), os, indent + 2);
        }
        break;
    }
    case ASTNodeKind::While: {
        auto wh = static_cast<const ASTWhile *>(stmt);
        printIndent(os, indent);
        os << "While\n";

        printIndent(os, indent + 1);
        os << "Cond:\n";
        dumpExpr(wh->condition.get(), os, indent + 2);

        printIndent(os, indent + 1);
        os << "Body:\n";
        dumpStmt(wh->body.get(), os, indent + 2);
        break;
    }
    case ASTNodeKind::Return: {
        auto ret = static_cast<const ASTReturn *>(stmt);
        printIndent(os, indent);
        os << "Return\n";
        if (ret->value) {
            dumpExpr(ret->value.get(), os, indent + 1);
        }
        break;
    }
    case ASTNodeKind::ExprStmt: {
        auto es = static_cast<const ASTExprStmt *>(stmt);
        printIndent(os, indent);
        os << "ExprStmt\n";
        if (es->expr) {
            dumpExpr(es->expr.get(), os, indent + 1);
        }
        break;
    }
    default:
        printIndent(os, indent);
        os << "UnknownStmt(kind=" << static_cast<int>(stmt->kind) << ")\n";
        break;
    }
}

// --- dump 表达式 ---

static void dumpExpr(const ASTExpression *expr, std::ostream &os, int indent) {
    switch (expr->kind) {
    case ASTNodeKind::BinaryExpr: {
        auto be = static_cast<const ASTBinaryExpr *>(expr);
        printIndent(os, indent);
        os << "BinaryExpr " << binaryOpToString(be->op) << "\n";
        dumpExpr(be->left.get(), os, indent + 1);
        dumpExpr(be->right.get(), os, indent + 1);
        break;
    }
    case ASTNodeKind::UnaryExpr: {
        auto ue = static_cast<const ASTUnaryExpr *>(expr);
        printIndent(os, indent);
        os << "UnaryExpr " << unaryOpToString(ue->op) << "\n";
        dumpExpr(ue->expr.get(), os, indent + 1);
        break;
    }
    case ASTNodeKind::IdentifierExpr: {
        auto id = static_cast<const ASTIdentifierExpr *>(expr);
        printIndent(os, indent);
        os << "Identifier " << id->name << "\n";
        break;
    }
    case ASTNodeKind::NumberExpr: {
        auto num = static_cast<const ASTNumberExpr *>(expr);
        printIndent(os, indent);
        os << "Number " << num->value << "\n";
        break;
    }
    case ASTNodeKind::CallExpr: {
        auto call = static_cast<const ASTCallExpr *>(expr);
        printIndent(os, indent);
        os << "Call " << call->callee << "\n";
        for (const auto &arg : call->arguments) {
            dumpExpr(arg.get(), os, indent + 1);
        }
        break;
    }
    case ASTNodeKind::AssignExpr: {
        auto asg = static_cast<const ASTAssignExpr *>(expr);
        printIndent(os, indent);
        os << "Assign " << asg->name << "\n";
        dumpExpr(asg->value.get(), os, indent + 1);
        break;
    }
    default:
        printIndent(os, indent);
        os << "UnknownExpr(kind=" << static_cast<int>(expr->kind) << ")\n";
        break;
    }
}