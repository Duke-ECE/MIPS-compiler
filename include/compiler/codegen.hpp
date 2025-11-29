/**
 * @file codegen.hpp
 * @brief assembly code generation from IR
 * 
 * This file defines the CodeGen class that translates an Intermediate Representation (IR)
 * into assembly code suitable for a target architecture (e.g., MIPS).
 * 
 * CodeGen class:
 * - generateAssembly: Main function to convert IRProgram to assembly code.
 * - translate: Helper function to translate individual IR instructions to assembly.
 * 
 */

#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "compiler/ir.hpp"
#include "isa/Registers.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <unordered_map>
#include <set>

class CodeGen {
public:
    CodeGen();

    // IR → Assembly text (vector<string>)
    std::vector<std::string> generateAssembly(const IRProgram &program);

private:
    // MIPS寄存器管理
    std::unordered_map<std::string, std::string> regMap;  // 变量名 → 寄存器名
    std::vector<std::string> availableTemps;              // 可用临时寄存器池
    std::vector<std::string> availableSaved;              // 可用保存寄存器池
    std::unordered_map<std::string, bool> usedCalleeSaved; // 使用的callee-saved寄存器
    
    int currentFrameSize;                                 // 当前函数栈帧大小

    // MIPS寄存器分配
    std::string allocateTemp();                           // 分配临时寄存器
    std::string allocateSaved();                          // 分配保存寄存器
    std::string allocateRegister(const std::string &var); // 为变量分配寄存器
    void releaseRegister(const std::string &reg);        // 释放寄存器
    
    // MIPS调用约定支持
    void generatePrologue(int frameSize, std::vector<std::string> &out);
    void generateEpilogue(int frameSize, std::vector<std::string> &out);
    void saveCallerSaved(std::vector<std::string> &out);
    void restoreCallerSaved(std::vector<std::string> &out);

    // 把 IRInstruction 翻译成多行 asm
    void translate(const IRInstruction &inst,
                   std::vector<std::string> &outAsm);

    // helpers
    std::string regName(int id) const;     // $rX (废弃，改用MIPS寄存器名)
    std::string labelName(const std::string &name) const;
    bool isMIPSRegister(const std::string &name) const;   // 检查是否为MIPS寄存器名
    void initRegisterPools();                             // 初始化寄存器池
};

#endif // CODEGEN_HPP
