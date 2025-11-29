#include "compiler/codegen.hpp"
#include <sstream>
#include <stdexcept>

CodeGen::CodeGen() : currentFrameSize(0) {
    initRegisterPools();
}

// MIPS寄存器分配策略
std::string CodeGen::allocateRegister(const std::string &var) {
    // 常量 "0" 直接使用 $r0
    if (var == "0" || var == "$r0" || var == "$zero") {
        return "$r0";
    }
    
    // 已经分配的寄存器
    auto it = regMap.find(var);
    if (it != regMap.end()) {
        return it->second;
    }
    
    // 为新变量分配临时寄存器
    std::string reg = allocateTemp();
    regMap[var] = reg;
    return reg;
}

std::string CodeGen::regName(int id) const {
    // 传统方法，保持兼容性
    return "$r" + std::to_string(id);
}

std::string CodeGen::labelName(const std::string &name) const {
    return name;
}

std::vector<std::string>
CodeGen::generateAssembly(const IRProgram &program) {
    std::vector<std::string> out;
    for (auto &inst : program.instructions)
        translate(inst, out);
    return out;
}

void CodeGen::translate(const IRInstruction &I,
                        std::vector<std::string> &out)
{
    const std::string &op = I.op;

    // --------------------
    // 函数标签
    // --------------------
    if (op == "label") {
        out.push_back(I.dst + ":");
        return;
    }
    
    // --------------------
    // 函数入口（生成prologue）
    // --------------------
    if (op == "enter") {
        currentFrameSize = std::stoi(I.dst);
        generatePrologue(currentFrameSize, out);
        return;
    }
    
    // --------------------
    // 函数出口（生成epilogue）
    // --------------------
    if (op == "leave") {
        generateEpilogue(currentFrameSize, out);
        return;
    }
    
    // --------------------
    // 从栈帧加载变量
    // --------------------
    if (op == "load") {
        std::string reg = allocateRegister(I.dst);
        out.push_back("lw " + reg + ", " + I.src1);
        return;
    }
    
    // --------------------
    // 存储变量到栈帧
    // --------------------
    if (op == "store") {
        std::string reg = allocateRegister(I.dst);
        out.push_back("sw " + reg + ", " + I.src1);
        return;
    }
    
    // --------------------
    // 参数传递
    // --------------------
    if (op == "param") {
        int paramIndex = std::stoi(I.src1);
        std::string srcReg = allocateRegister(I.dst);
        
        if (paramIndex < 4) {
            // 前4个参数使用$a0-$a3
            std::string argReg = "$a" + std::to_string(paramIndex);
            out.push_back("addi " + argReg + ", " + srcReg + ", 0");
        } else {
            // 第5个及以后的参数放在栈上 (word addressed)
            int offset = (paramIndex - 4) + 1; // 从栈顶往下，word偏移
            out.push_back("sw " + srcReg + ", " + std::to_string(offset) + "($sp)");
        }
        return;
    }

    // --------------------
    // Jump
    // --------------------
    if (op == "jmp") {
        out.push_back("j " + labelName(I.dst));
        return;
    }

    // --------------------
    // Return
    // --------------------
    if (op == "ret") {
        if (!I.dst.empty()) {
            // 将返回值放入$v0
            std::string srcReg = allocateRegister(I.dst);
            out.push_back("addi $v0, " + srcReg + ", 0");
        }
        // 注意：不在这里直接返回，leave指令会处理返回
        return;
    }

    // --------------------
    // MOV
    // --------------------
    if (op == "mov") {
        std::string rdReg = allocateRegister(I.dst);

        bool isImm = true;
        for (char c : I.src1)
            if (!isdigit(c) && c!='-') isImm = false;

        if (isImm) {
            out.push_back("addi " + rdReg + ", $r0, " + I.src1);
        } else {
            std::string rsReg = allocateRegister(I.src1);
            out.push_back("addi " + rdReg + ", " + rsReg + ", 0");
        }
        return;
    }

    // --------------------
    // add/sub/and/or
    // --------------------
    if (op == "add" || op == "sub" ||
        op == "and" || op == "or")
    {
        std::string rdReg = allocateRegister(I.dst);
        std::string rsReg = allocateRegister(I.src1);
        std::string rtReg = allocateRegister(I.src2);

        out.push_back(op + " " + rdReg + ", "
                             + rsReg + ", "
                             + rtReg);
        return;
    }

    // --------------------
    // mul/div (software)
    // --------------------
    if (op == "mul" || op == "div") {
        std::string rdReg = allocateRegister(I.dst);
        std::string aReg  = allocateRegister(I.src1);
        std::string bReg  = allocateRegister(I.src2);

        static int lbl = 0;
        std::string L_loop = "LMD_LOOP_" + std::to_string(lbl);
        std::string L_end  = "LMD_END_"  + std::to_string(lbl);
        lbl++;

        out.push_back("addi " + rdReg + ", $r0, 0");

        if (op == "mul") {
            out.push_back(L_loop + ":");
            out.push_back("beq " + bReg + ", $r0, " + L_end);
            out.push_back("add " + rdReg + ", " + rdReg + ", " + aReg);
            out.push_back("addi " + bReg + ", " + bReg + ", -1");
            out.push_back("j " + L_loop);
            out.push_back(L_end + ":");
        } else {
            out.push_back(L_loop + ":");
            out.push_back("bgt " + bReg + ", " + aReg + ", " + L_end);
            out.push_back("sub " + aReg + ", " + aReg + ", " + bReg);
            out.push_back("addi " + rdReg + ", " + rdReg + ", 1");
            out.push_back("j " + L_loop);
            out.push_back(L_end + ":");
        }

        return;
    }

    // --------------------
    // Unary neg
    // --------------------
    if (op == "neg") {
        std::string rdReg = allocateRegister(I.dst);
        std::string rsReg = allocateRegister(I.src1);
        out.push_back("sub " + rdReg + ", $r0, " + rsReg);
        return;
    }

    // --------------------
    // Unary not
    // --------------------
    if (op == "not") {
        std::string rdReg = allocateRegister(I.dst);
        std::string rsReg = allocateRegister(I.src1);

        static int lbl = 0;
        std::string L_true = "LNOT_T_" + std::to_string(lbl);
        std::string L_end  = "LNOT_E_" + std::to_string(lbl);
        lbl++;

        out.push_back("beq " + rsReg + ", $r0, " + L_true);
        out.push_back("addi " + rdReg + ", $r0, 0");
        out.push_back("j " + L_end);
        out.push_back(L_true + ":");
        out.push_back("addi " + rdReg + ", $r0, 1");
        out.push_back(L_end + ":");
        return;
    }

    // --------------------
    // IR-level comparison → assembly
    // --------------------
    if (op=="lt"||op=="le"||op=="gt"||op=="ge"||op=="eq"||op=="ne")
    {
        std::string rdReg = allocateRegister(I.dst);
        std::string aReg  = allocateRegister(I.src1);
        std::string bReg  = allocateRegister(I.src2);

        static int lbl = 0;
        std::string LT = "LCMP_T_" + std::to_string(lbl);
        std::string LE = "LCMP_E_" + std::to_string(lbl);
        lbl++;

        if (op=="lt") {
            out.push_back("bgt " + bReg + ", " + aReg + ", " + LT);
        }
        else if (op=="gt") {
            out.push_back("bgt " + aReg + ", " + bReg + ", " + LT);
        }
        else if (op=="eq") {
            out.push_back("beq " + aReg + ", " + bReg + ", " + LT);
        }
        else if (op=="ne") {
            out.push_back("beq " + aReg + ", " + bReg + ", " + LE);
        }
        else if (op=="le") {
            out.push_back("bgt " + aReg + ", " + bReg + ", " + LE);
            out.push_back("j " + LT);
        }
        else if (op=="ge") {
            out.push_back("bgt " + bReg + ", " + aReg + ", " + LE);
            out.push_back("j " + LT);
        }

        out.push_back("addi " + rdReg + ", $r0, 0");
        out.push_back("j " + LE);
        out.push_back(LT + ":");
        out.push_back("addi " + rdReg + ", $r0, 1");
        out.push_back(LE + ":");
        return;
    }

    // --------------------
    // IR-level beq(rd, rs, label)
    // --------------------
    if (op == "beq") {
        std::string rdReg = allocateRegister(I.dst);
        std::string rsReg = (I.src1 == "0") ? "$r0" : allocateRegister(I.src1);
        out.push_back("beq " + rdReg + ", " + rsReg + ", " + I.src2);
        return;
    }

    // --------------------
    // IR-level bgt
    // --------------------
    if (op == "bgt") {
        std::string rdReg = allocateRegister(I.dst);
        std::string rsReg = allocateRegister(I.src1);
        out.push_back("bgt " + rdReg + ", " + rsReg + ", " + I.src2);
        return;
    }

    // --------------------
    // Optional: IR-level brz cond, label
    // --------------------
    if (op == "brz") {
        std::string rReg = allocateRegister(I.dst);
        out.push_back("beq " + rReg + ", $r0, " + I.src1);
        return;
    }

    // --------------------
    // 函数调用
    // --------------------
    if (op == "call") {
        // 保存caller-saved寄存器
        saveCallerSaved(out);
        
        // 调用函数
        out.push_back("jal " + I.src1);
        
        // 恢复caller-saved寄存器
        restoreCallerSaved(out);
        
        // 获取返回值到目标寄存器
        std::string dstReg = allocateRegister(I.dst);
        out.push_back("addi " + dstReg + ", $v0, 0");
        return;
    }

    // --------------------
    // input / output
    // --------------------
    if (op == "input") {
        std::string rdReg = allocateRegister(I.dst);
        out.push_back("input " + rdReg);
        return;
    }
    if (op == "output") {
        std::string rdReg = allocateRegister(I.dst);
        out.push_back("output " + rdReg);
        return;
    }

    throw std::runtime_error("Unknown IR op: " + op);
}

// MIPS调用约定支持方法实现

void CodeGen::initRegisterPools() {
    // 初始化临时寄存器池 ($t0-$t9)
    availableTemps = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"};
    
    // 初始化保存寄存器池 ($s0-$s7)  
    availableSaved = {"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"};
}

std::string CodeGen::allocateTemp() {
    if (!availableTemps.empty()) {
        std::string reg = availableTemps.back();
        availableTemps.pop_back();
        return reg;
    }
    
    // 临时寄存器用完，需要spill（简化处理：使用$t0）
    return "$t0";
}

std::string CodeGen::allocateSaved() {
    if (!availableSaved.empty()) {
        std::string reg = availableSaved.back();
        availableSaved.pop_back();
        usedCalleeSaved[reg] = true;
        return reg;
    }
    
    // 保存寄存器用完，回退到临时寄存器
    return allocateTemp();
}

void CodeGen::releaseRegister(const std::string &reg) {
    // 将寄存器放回可用池
    if (reg.find("$t") == 0) {
        availableTemps.push_back(reg);
    } else if (reg.find("$s") == 0) {
        availableSaved.push_back(reg);
        usedCalleeSaved[reg] = false;
    }
}

void CodeGen::generatePrologue(int frameSize, std::vector<std::string> &out) {
    // MIPS函数入口序列 (word-addressed)
    out.push_back("# Function prologue");
    out.push_back("addi $sp, $sp, -" + std::to_string(frameSize));  // 分配栈帧 (word偏移)
    out.push_back("sw $fp, 1($sp)");                               // 保存旧fp (word addressed)
    out.push_back("addi $fp, $sp, 0");                            // 设置新fp
    out.push_back("sw $ra, 2($sp)");                              // 保存返回地址
    
    // 保存使用的callee-saved寄存器
    int offset = 3;
    for (const auto &pair : usedCalleeSaved) {
        if (pair.second) {
            out.push_back("sw " + pair.first + ", " + std::to_string(offset) + "($sp)");
            offset++;
        }
    }
}

void CodeGen::generateEpilogue(int frameSize, std::vector<std::string> &out) {
    // 恢复callee-saved寄存器
    int offset = 3;
    for (const auto &pair : usedCalleeSaved) {
        if (pair.second) {
            out.push_back("lw " + pair.first + ", " + std::to_string(offset) + "($fp)");
            offset++;
        }
    }
    
    // MIPS函数退出序列 (word-addressed)
    out.push_back("# Function epilogue");
    out.push_back("lw $ra, 2($fp)");                              // 恢复返回地址
    out.push_back("lw $fp, 1($fp)");                              // 恢复旧fp
    out.push_back("addi $sp, $sp, " + std::to_string(frameSize)); // 释放栈帧 (word偏移)
    out.push_back("jr $ra");                                      // 返回
}

void CodeGen::saveCallerSaved(std::vector<std::string> &out) {
    // 保存caller-saved寄存器 ($t0-$t9)
    out.push_back("# Save caller-saved registers");
    for (int i = 0; i < 10; ++i) {
        out.push_back("sw $t" + std::to_string(i) + ", -" + std::to_string(i + 1) + "($sp)");
    }
    out.push_back("addi $sp, $sp, -10");  // 调整栈指针 (10 words)
}

void CodeGen::restoreCallerSaved(std::vector<std::string> &out) {
    // 恢复caller-saved寄存器
    out.push_back("addi $sp, $sp, 10");   // 恢复栈指针 (10 words)
    out.push_back("# Restore caller-saved registers");
    for (int i = 9; i >= 0; --i) {
        out.push_back("lw $t" + std::to_string(i) + ", -" + std::to_string(i + 1) + "($sp)");
    }
}

bool CodeGen::isMIPSRegister(const std::string &name) const {
    return name.length() > 0 && name[0] == '$';
}
