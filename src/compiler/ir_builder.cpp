#include "compiler/ir_builder.hpp"
#include <stdexcept>
#include <sstream>

IRBuilder::IRBuilder()
    : tempCounter(0), frameSize(0), localVarOffset(0), hasExplicitReturn(false)
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
    // 重置函数级状态
    frameSize = 0;
    localVarOffset = 0;
    localVars.clear();
    hasExplicitReturn = false;
    
    program.emit({ "label", node->name, "", "" });
    
    // 计算栈帧大小（预扫描）
    calculateFrameSize(node);
    
    // 生成函数入口
    program.emit({ "enter", std::to_string(frameSize), "", "" });
    
    // 处理参数（前4个通过寄存器传递，其余在栈上）
    for (size_t i = 0; i < node->paramNames.size(); ++i) {
        const std::string &param = node->paramNames[i];
        if (i < 4) {
            // 参数通过$a0-$a3传递，需要存储到栈帧
            int offset = allocateLocalVar(param);
            program.emit({ "store", "$a" + std::to_string(i), std::to_string(offset), "" });
        } else {
            // 参数已在栈上，记录其位置
            int offset = 1 + (i - 4); // 参数在fp之上（word addressed）
            localVars[param] = offset;
        }
    }
    
    genBlock(node->body.get());
    
    // 生成函数出口（如果没有显式return）
    if (!hasExplicitReturn) {
        program.emit({ "leave", "", "", "" });
    }
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
    // 为局部变量分配栈空间
    int offset = allocateLocalVar(node->name);
    
    if (node->initExpr) {
        std::string rhs = genExpr(node->initExpr.get());
        program.emit({ "store", rhs, std::to_string(offset), "" });
    }
}

void IRBuilder::genIf(const ASTIf *node) {
    std::string cond = genExpr(node->condition.get());

    std::string labelElse = "L" + std::to_string(tempCounter++);
    std::string labelEnd  = "L" + std::to_string(tempCounter++);

    // 替换 brz → beq cond, 0, labelElse
    program.emit({ "beq", cond, "0", labelElse });

    genStmt(node->thenBranch.get());
    program.emit({ "jmp", labelEnd, "", "" });

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

    // 替换 brz → beq cond, 0, labelEnd
    program.emit({ "beq", cond, "0", labelEnd });

    genStmt(node->body.get());
    program.emit({ "jmp", labelStart, "", "" });

    program.emit({ "label", labelEnd, "", "" });
}

void IRBuilder::genReturn(const ASTReturn *node) {
    hasExplicitReturn = true;
    if (node->value) {
        std::string val = genExpr(node->value.get());
        program.emit({ "ret", val, "", "" });
    } else {
        program.emit({ "ret", "", "", "" });
    }
    program.emit({ "leave", "", "", "" });
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
    } else {
        program.emit({ "not", dst, src, "" });
    }

    return dst;
}

std::string IRBuilder::genIdentifier(const ASTIdentifierExpr *node) {
    // 检查是否为局部变量
    auto it = localVars.find(node->name);
    if (it != localVars.end()) {
        // 从栈帧加载变量
        std::string temp = newTemp();
        program.emit({ "load", temp, std::to_string(it->second), "" });
        return temp;
    }
    
    // 可能是全局变量或函数名
    return node->name;
}

std::string IRBuilder::genNumber(const ASTNumberExpr *node) {
    std::string dst = newTemp();
    program.emit({ "mov", dst, std::to_string(node->value), "" });
    return dst;
}

std::string IRBuilder::genCall(const ASTCallExpr *node) {
    // 生成参数表达式
    std::vector<std::string> args;
    for (auto &arg : node->arguments) {
        args.push_back(genExpr(arg.get()));
    }
    
    // 按MIPS约定传递参数
    for (size_t i = 0; i < args.size(); ++i) {
        program.emit({ "param", args[i], std::to_string(i), "" });
    }
    
    // 生成函数调用
    std::string dst = newTemp();
    program.emit({ "call", dst, node->callee, std::to_string(args.size()) });
    
    return dst;
}

std::string IRBuilder::genAssign(const ASTAssignExpr *node) {
    std::string rhs = genExpr(node->value.get());
    
    // 检查是否为局部变量
    auto it = localVars.find(node->name);
    if (it != localVars.end()) {
        // 存储到栈帧
        program.emit({ "store", rhs, std::to_string(it->second), "" });
        return rhs;
    }
    
    // 全局变量或其他情况
    program.emit({ "mov", node->name, rhs, "" });
    return node->name;
}

// MIPS栈帧管理方法实现
int IRBuilder::allocateLocalVar(const std::string &varName) {
    localVarOffset -= 1; // 每个变量占用1 word
    localVars[varName] = localVarOffset;
    return localVarOffset;
}

void IRBuilder::calculateFrameSize(const ASTFunction *func) {
    // 简化实现：为每个局部变量预分配4字节
    // 实际实现需要递归扫描AST节点计算所需空间
    
    // 基础栈帧：保存$fp(1 word) + 保存$ra(1 word) = 2 words
    frameSize = 2;
    
    // 为函数参数预留空间（如果超过4个参数）
    if (func->paramNames.size() > 4) {
        frameSize += (func->paramNames.size() - 4);
    }
    
    // 为局部变量预留空间（简化：假设最多16个局部变量）
    frameSize += 16; // 16 words局部变量空间
    
    // 为caller-saved寄存器预留空间（$t0-$t9 = 10个寄存器）
    frameSize += 10; // 10 words临时寄存器保存空间
    
    // 对齐到2 word边界（8字节）
    frameSize = (frameSize + 1) & ~1;
}
