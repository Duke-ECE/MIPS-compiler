#include "compiler/codegen.hpp"
#include <sstream>
#include <stdexcept>

CodeGen::CodeGen() : currentFrameSize(0) {
}

// ========================================================
// 寄存器分配：MIPS 风格
// ========================================================

std::string CodeGen::allocateRegister(const std::string &var) {
    if (var == "0" || var == "$r0" || var == "$zero")
        return "$zero";

    // 如果已经是寄存器名（以$开头），直接返回
    if (!var.empty() && var[0] == '$') {
        return var;
    }

    // 查询寄存器分配器的结果
    if (regAllocator.hasLocation(var)) {
        const auto &loc = regAllocator.getLocation(var);
        if (loc.inReg) {
            return loc.reg;
        }
        // 变量已被溢出到栈，但当前简化实现中我们仍然需要一个寄存器来访问它
        // 使用一个临时寄存器（这是一个简化，真正的实现应该在之前reload）
        // 为了简化，我们为溢出的变量也分配一个临时寄存器
        if (regMap.find(var) == regMap.end()) {
            // 使用栈位置对应的临时寄存器（循环使用t0-t9）
            int idx = (-loc.stackOffset - 3) % 10;
            regMap[var] = "$t" + std::to_string(idx);
        }
        return regMap[var];
    }

    // 如果有本地映射，使用本地映射
    auto it = regMap.find(var);
    if (it != regMap.end())
        return it->second;

    // 变量没有分配，这是一个错误
    throw std::runtime_error("Variable " + var + " has no register allocation");
}

std::string CodeGen::regName(int id) const {
    return "$r" + std::to_string(id);
}

std::string CodeGen::labelName(const std::string &name) const {
    return name;
}

// ========================================================
// 主入口：翻译 IR → Assembly
// ========================================================

std::vector<std::string>
CodeGen::generateAssembly(const IRProgram &program) {
    // 使用寄存器分配器进行寄存器分配和spill/reload
    allocatedProgram = regAllocator.allocate(program);
    
    // 生成汇编代码
    std::vector<std::string> out;
    for (auto &inst : allocatedProgram.instructions) {
        translate(inst, out);
    }
    
    return out;
}

// ========================================================
// IR 指令翻译核心
// ========================================================
void CodeGen::translate(const IRInstruction &I,
                        std::vector<std::string> &out)
{
    const std::string &op = I.op;

    // ------------------------------------
    // 函数标签
    // ------------------------------------
    if (op == "label") {
        out.push_back(I.dst + ":");
        return;
    }

    // ------------------------------------
    // enter（函数入口）
    // ------------------------------------
    if (op == "enter") {
        currentFrameSize = std::stoi(I.dst);      // frameSize 是 word 数
        generatePrologue(currentFrameSize, out);
        return;
    }

    // ------------------------------------
    // leave（函数出口）
    // ------------------------------------
    if (op == "leave") {
        generateEpilogue(currentFrameSize, out);
        return;
    }

    // ------------------------------------
    // load: load dst, offset  → lw dst, offset($fp)
    // ------------------------------------
    if (op == "load") {
        std::string rd = allocateRegister(I.dst);
        out.push_back("lw " + rd + ", " + I.src1 + "($fp)");
        return;
    }

    // ------------------------------------
    // store: store src, offset → sw src, offset($fp)
    // ------------------------------------
    if (op == "store") {
        std::string rs = allocateRegister(I.dst);
        out.push_back("sw " + rs + ", " + I.src1 + "($fp)");
        return;
    }

    // ------------------------------------
    // 参数传递 param x, index
    // ------------------------------------
    if (op == "param") {
        int idx = std::stoi(I.src1);
        std::string r = allocateRegister(I.dst);

        if (idx < 4) {
            std::string a = "$a" + std::to_string(idx);
            out.push_back("addi " + a + ", " + r + ", 0");
        } else {
            int spillOffset = (idx - 4) + 1;  // word addressed
            out.push_back("sw " + r + ", " + std::to_string(spillOffset) + "($sp)");
        }
        return;
    }

    // ------------------------------------
    // jmp
    // ------------------------------------
    if (op == "jmp") {
        out.push_back("j " + I.dst);
        return;
    }

    // ------------------------------------
    // ret
    // ------------------------------------
    if (op == "ret") {
        if (!I.dst.empty()) {
            std::string r = allocateRegister(I.dst);
            out.push_back("addi $v0, " + r + ", 0");
        }
        return;
    }

    // ------------------------------------
    // mov
    // ------------------------------------
    if (op == "mov") {
        std::string rd = allocateRegister(I.dst);

        bool imm = true;
        for (char c : I.src1)
            if (!isdigit(c) && c != '-') imm = false;

        if (imm) {
            out.push_back("addi " + rd + ", $zero, " + I.src1);
        } else {
            std::string rs = allocateRegister(I.src1);
            out.push_back("addi " + rd + ", " + rs + ", 0");
        }
        return;
    }

    // ------------------------------------
    // add/sub/and/or
    // ------------------------------------
    if (op=="add" || op=="sub" || op=="and" || op=="or") {
        std::string rd = allocateRegister(I.dst);
        std::string rs = allocateRegister(I.src1);
        std::string rt = allocateRegister(I.src2);
        out.push_back(op + " " + rd + ", " + rs + ", " + rt);
        return;
    }

    // ------------------------------------
    // mul/div (软件实现)
    // ------------------------------------
    if (op=="mul" || op=="div") {
        std::string rd = allocateRegister(I.dst);
        std::string ra = allocateRegister(I.src1);
        std::string rb = allocateRegister(I.src2);

        static int L = 0;
        std::string loop = "LMD_LOOP_" + std::to_string(L);
        std::string end  = "LMD_END_"  + std::to_string(L);
        L++;

        out.push_back("addi " + rd + ", $zero, 0");

        if (op=="mul") {
            out.push_back(loop + ":");
            out.push_back("beq " + rb + ", $zero, " + end);
            out.push_back("add " + rd + ", " + rd + ", " + ra);
            out.push_back("addi " + rb + ", " + rb + ", -1");
            out.push_back("j " + loop);
            out.push_back(end + ":");
        } else {
            out.push_back(loop + ":");
            out.push_back("bgt " + rb + ", " + ra + ", " + end);
            out.push_back("sub " + ra + ", " + ra + ", " + rb);
            out.push_back("addi " + rd + ", " + rd + ", 1");
            out.push_back("j " + loop);
            out.push_back(end + ":");
        }
        return;
    }

    // ------------------------------------
    // unary neg
    // ------------------------------------
    if (op == "neg") {
        std::string rd = allocateRegister(I.dst);
        std::string rs = allocateRegister(I.src1);
        out.push_back("sub " + rd + ", $zero, " + rs);
        return;
    }

    // ------------------------------------
    // unary not
    // ------------------------------------
    if (op == "not") {
        std::string rd = allocateRegister(I.dst);
        std::string rs = allocateRegister(I.src1);

        static int L=0;
        std::string T = "LNOT_T_" + std::to_string(L);
        std::string E = "LNOT_E_" + std::to_string(L);
        L++;

        out.push_back("beq " + rs + ", $zero, " + T);
        out.push_back("addi " + rd + ", $zero, 0");
        out.push_back("j " + E);
        out.push_back(T + ":");
        out.push_back("addi " + rd + ", $zero, 1");
        out.push_back(E + ":");
        return;
    }

    // ------------------------------------
    // Comparisons
    // ------------------------------------
    if (op=="lt"||op=="le"||op=="gt"||op=="ge"||op=="eq"||op=="ne")
    {
        std::string rd = allocateRegister(I.dst);
        std::string a  = allocateRegister(I.src1);
        std::string b  = allocateRegister(I.src2);

        static int L=0;
        std::string T = "LCMP_T_" + std::to_string(L);
        std::string E = "LCMP_E_" + std::to_string(L);
        L++;

        if      (op=="lt") out.push_back("bgt " + b + ", " + a + ", " + T);
        else if (op=="gt") out.push_back("bgt " + a + ", " + b + ", " + T);
        else if (op=="eq") out.push_back("beq " + a + ", " + b + ", " + T);
        else if (op=="ne") out.push_back("beq " + a + ", " + b + ", " + E);
        else if (op=="le") { out.push_back("bgt " + a + ", " + b + ", " + E); out.push_back("j " + T); }
        else if (op=="ge") { out.push_back("bgt " + b + ", " + a + ", " + E); out.push_back("j " + T); }

        out.push_back("addi " + rd + ", $zero, 0");
        out.push_back("j " + E);
        out.push_back(T + ":");
        out.push_back("addi " + rd + ", $zero, 1");
        out.push_back(E + ":");
        return;
    }

    // ------------------------------------
    // beq
    // ------------------------------------
    if (op == "beq") {
        std::string rd = allocateRegister(I.dst);
        std::string rs = (I.src1=="0" ? "$zero" : allocateRegister(I.src1));
        out.push_back("beq " + rd + ", " + rs + ", " + I.src2);
        return;
    }

    // ------------------------------------
    // bgt
    // ------------------------------------
    if (op == "bgt") {
        std::string rd = allocateRegister(I.dst);
        std::string rs = allocateRegister(I.src1);
        out.push_back("bgt " + rd + ", " + rs + ", " + I.src2);
        return;
    }

    // ------------------------------------
    // call
    // ------------------------------------
    if (op == "call") {

        saveCallerSaved(out);

        out.push_back("jal " + I.src1);

        restoreCallerSaved(out);

        std::string rd = allocateRegister(I.dst);
        out.push_back("addi " + rd + ", $v0, 0");
        return;
    }

    // ------------------------------------
    // input/output
    // ------------------------------------
    if (op=="input") {
        std::string rd = allocateRegister(I.dst);
        out.push_back("input " + rd);
        return;
    }
    if (op=="output") {
        std::string rs = allocateRegister(I.dst);
        out.push_back("output " + rs);
        return;
    }

    throw std::runtime_error("Unknown IR op: " + op);
}

// ========================================================
// Register Pool Initialization
// ========================================================
// ========================================================
// Prologue / Epilogue
// ========================================================

void CodeGen::generatePrologue(int frameSize,
                               std::vector<std::string> &out)
{
    out.push_back("# Function prologue");

    out.push_back("addi $sp, $sp, -" + std::to_string(frameSize));

    out.push_back("sw $fp, 1($sp)");
    out.push_back("addi $fp, $sp, 0");
    out.push_back("sw $ra, 2($sp)");

    // 保存使用的callee-saved寄存器
    int offs = 3;
    const auto &usedCalleeSaved = regAllocator.getUsedCalleeSaved();
    std::vector<std::string> savedOrder = {
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"
    };
    
    for (auto &reg : savedOrder) {
        auto it = usedCalleeSaved.find(reg);
        if (it != usedCalleeSaved.end() && it->second) {
            out.push_back("sw " + reg + ", " + std::to_string(offs) + "($sp)");
            offs++;
        }
    }
}

void CodeGen::generateEpilogue(int frameSize,
                               std::vector<std::string> &out)
{
    // 恢复使用的callee-saved寄存器
    int offs = 3;
    const auto &usedCalleeSaved = regAllocator.getUsedCalleeSaved();
    std::vector<std::string> savedOrder = {
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"
    };
    
    for (auto &reg : savedOrder) {
        auto it = usedCalleeSaved.find(reg);
        if (it != usedCalleeSaved.end() && it->second) {
            out.push_back("lw " + reg + ", " + std::to_string(offs) + "($fp)");
            offs++;
        }
    }

    out.push_back("# Function epilogue");
    out.push_back("lw $ra, 2($fp)");
    out.push_back("lw $fp, 1($fp)");
    out.push_back("addi $sp, $sp, " + std::to_string(frameSize));
    out.push_back("jr $ra");
}

// ========================================================
// Caller-saved Registers
// ========================================================

void CodeGen::saveCallerSaved(std::vector<std::string> &out) {
    out.push_back("# Save caller-saved");
    out.push_back("addi $sp, $sp, -10"); // reserve 10 words

    for (int i=0; i<10; ++i)
        out.push_back("sw $t" + std::to_string(i) + ", " + std::to_string(i+1) + "($sp)");
}

void CodeGen::restoreCallerSaved(std::vector<std::string> &out) {
    for (int i=0; i<10; ++i)
        out.push_back("lw $t" + std::to_string(i) + ", " + std::to_string(i+1) + "($sp)");

    out.push_back("addi $sp, $sp, 10");
}

bool CodeGen::isMIPSRegister(const std::string &name) const {
    return !name.empty() && name[0] == '$';
}
