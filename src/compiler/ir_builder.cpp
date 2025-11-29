#include "compiler/ir_builder.hpp"
#include <stdexcept>
#include <sstream>

IRBuilder::IRBuilder()
    : tempCounter(0)
{
}

std::string IRBuilder::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

IRProgram IRBuilder::generateIR(const ASTNode *root) {
    program.instructions.clear();

    if (root->kind != ASTNodeKind::Program) {
        throw std::runtime_error("IRBuilder: root must be Program");
    }

    genProgram(static_cast<const ASTProgram*>(root));
    return program;
}

// =====================================================
// Program
// =====================================================
void IRBuilder::genProgram(const ASTProgram *node) {
    for (auto &func : node->functions) {
        genFunction(func.get());
    }
}

void IRBuilder::genFunction(const ASTFunction *node) {
    // 函数入口标签
    program.emit({ "label", node->name, "", "" });

    // 生成函数体
    genBlock(node->body.get());
}

void IRBuilder::genBlock(const ASTBlock *node) {
    for (auto &stmt : node->statements) {
        genStmt(stmt.get());
    }
}

// =====================================================
// Statements
// =====================================================
void IRBuilder::genStmt(const ASTStatement *stmt) {
    switch (stmt->kind) {
        case ASTNodeKind::VarDecl:
            genVarDecl(static_cast<const ASTVarDecl*>(stmt));
            break;
        case ASTNodeKind::If:
            genIf(static_cast<const ASTIf*>(stmt));
            break;
        case ASTNodeKind::While:
            genWhile(static_cast<const ASTWhile*>(stmt));
            break;
        case ASTNodeKind::Return:
            genReturn(static_cast<const ASTReturn*>(stmt));
            break;
        case ASTNodeKind::ExprStmt:
            genExprStmt(static_cast<const ASTExprStmt*>(stmt));
            break;
        case ASTNodeKind::Block:
            genBlock(static_cast<const ASTBlock*>(stmt));
            break;
        default:
            throw std::runtime_error("IRBuilder: unknown statement kind");
    }
}

void IRBuilder::genVarDecl(const ASTVarDecl *node) {
    if (node->initExpr) {
        std::string rhs = genExpr(node->initExpr.get());
        program.emit({ "mov", node->name, rhs, "" });
    }
}

void IRBuilder::genIf(const ASTIf *node) {
    std::string cond = genExpr(node->condition.get());

    std::string labelElse = "L" + std::to_string(tempCounter++);
    std::string labelEnd  = "L" + std::to_string(tempCounter++);

    // if not cond goto else
    program.emit({ "brz", cond, labelElse, "" }); // brz x L  = if x==0 goto L

    genStmt(node->thenBranch.get());
    program.emit({ "jmp", labelEnd, "", "" });

    // else block
    program.emit({ "label", labelElse, "", "" });
    if (node->elseBranch) {
        genStmt(node->elseBranch.get());
    }

    program.emit({ "label", labelEnd, "", "" });
}

void IRBuilder::genWhile(const ASTWhile *node) {
    std::string labelStart = "L" + std::to_string(tempCounter++);
    std::string labelEnd   = "L" + std::to_string(tempCounter++);

    program.emit({ "label", labelStart, "", "" });

    std::string cond = genExpr(node->condition.get());
    program.emit({ "brz", cond, labelEnd, "" });

    genStmt(node->body.get());
    program.emit({ "jmp", labelStart, "", "" });

    program.emit({ "label", labelEnd, "", "" });
}

void IRBuilder::genReturn(const ASTReturn *node) {
    if (node->value) {
        std::string val = genExpr(node->value.get());
        program.emit({ "ret", val, "", "" });
    } else {
        program.emit({ "ret", "", "", "" });
    }
}

void IRBuilder::genExprStmt(const ASTExprStmt *node) {
    genExpr(node->expr.get());
}

// =====================================================
// Expressions
// =====================================================
std::string IRBuilder::genExpr(const ASTExpression *expr) {
    switch (expr->kind) {
        case ASTNodeKind::BinaryExpr:
            return genBinary(static_cast<const ASTBinaryExpr*>(expr));
        case ASTNodeKind::UnaryExpr:
            return genUnary(static_cast<const ASTUnaryExpr*>(expr));
        case ASTNodeKind::IdentifierExpr:
            return genIdentifier(static_cast<const ASTIdentifierExpr*>(expr));
        case ASTNodeKind::NumberExpr:
            return genNumber(static_cast<const ASTNumberExpr*>(expr));
        case ASTNodeKind::CallExpr:
            return genCall(static_cast<const ASTCallExpr*>(expr));
        case ASTNodeKind::AssignExpr:
            return genAssign(static_cast<const ASTAssignExpr*>(expr));
        default:
            throw std::runtime_error("IRBuilder: unknown expression kind");
    }
}

std::string IRBuilder::genBinary(const ASTBinaryExpr *node) {
    std::string lhs = genExpr(node->left.get());
    std::string rhs = genExpr(node->right.get());
    std::string dst = newTemp();

    std::string op;
    switch (node->op) {
        case ASTBinaryOpKind::Add: op = "add"; break;
        case ASTBinaryOpKind::Sub: op = "sub"; break;
        case ASTBinaryOpKind::Mul: op = "mul"; break;
        case ASTBinaryOpKind::Div: op = "div"; break;
        case ASTBinaryOpKind::Less: op = "lt"; break;
        case ASTBinaryOpKind::LessEqual: op = "le"; break;
        case ASTBinaryOpKind::Greater: op = "gt"; break;
        case ASTBinaryOpKind::GreaterEqual: op = "ge"; break;
        case ASTBinaryOpKind::Equal: op = "eq"; break;
        case ASTBinaryOpKind::NotEqual: op = "ne"; break;
    }

    program.emit({ op, dst, lhs, rhs });
    return dst;
}

std::string IRBuilder::genUnary(const ASTUnaryExpr *node) {
    std::string src = genExpr(node->expr.get());
    std::string dst = newTemp();

    if (node->op == ASTUnaryOpKind::Neg) {
        program.emit({ "neg", dst, src, "" });
    } else { // !
        program.emit({ "not", dst, src, "" });
    }

    return dst;
}

std::string IRBuilder::genIdentifier(const ASTIdentifierExpr *node) {
    return node->name;
}

std::string IRBuilder::genNumber(const ASTNumberExpr *node) {
    std::string dst = newTemp();
    program.emit({ "mov", dst, std::to_string(node->value), "" });
    return dst;
}

std::string IRBuilder::genCall(const ASTCallExpr *node) {
    // 先生成参数
    std::vector<std::string> argTemps;
    for (auto &arg : node->arguments) {
        argTemps.push_back(genExpr(arg.get()));
    }

    // call 指令格式：call dst funcName argCount ...
    std::string dst = newTemp();

    IRInstruction inst;
    inst.op = "call";
    inst.dst = dst;
    inst.src1 = node->callee;
    inst.src2 = std::to_string(argTemps.size());  // arg count
    program.emit(inst);

    return dst;
}

std::string IRBuilder::genAssign(const ASTAssignExpr *node) {
    std::string rhs = genExpr(node->value.get());
    program.emit({ "mov", node->name, rhs, "" });
    return node->name;
}
