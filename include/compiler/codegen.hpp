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
#include <string>
#include <vector>
#include <unordered_map>

class CodeGen {
public:
    CodeGen();

    // IR → Assembly text (vector<string>)
    std::vector<std::string> generateAssembly(const IRProgram &program);

private:
    // 变量名 → 寄存器编号
    std::unordered_map<std::string, int> regMap;

    // 给新变量分配寄存器
    int allocateRegister(const std::string &var);

    // 把 IRInstruction 翻译成多行 asm
    void translate(const IRInstruction &inst,
                   std::vector<std::string> &outAsm);

    // helpers
    std::string regName(int id) const;     // $rX
    std::string labelName(const std::string &name) const;
};

#endif // CODEGEN_HPP
