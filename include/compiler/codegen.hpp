/**
 * @file codegen.hpp
 * @brief assembly code generation from IR
 * 
 * This file defines the CodeGen class that translates an Intermediate Representation (IR)
 * into assembly code suitable for a target architecture (e.g., MIPS).
 * 
 */

#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include "compiler/ir.hpp"
#include "compiler/reg_allocator.hpp"
#include "isa/Registers.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <set>

class CodeGen {
public:
    CodeGen();

    // IR → Assembly text (vector<string>)
    std::vector<std::string> generateAssembly(const IRProgram &program);

private:
    // -----------------------------
    // Register allocation state
    // -----------------------------
    RegisterAllocator regAllocator;                        // 寄存器分配器
    std::unordered_map<std::string, std::string> regMap;   // variable → register
    IRProgram allocatedProgram;                             // 经过寄存器分配的IR

    int currentFrameSize;                                  // stack frame size


    // -----------------------------
    // Register allocators (保留用于局部寄存器查询)
    // -----------------------------
    std::string allocateRegister(const std::string &var);  // var → register


    // -----------------------------
    // Calling-convention support
    // -----------------------------
    void generatePrologue(int frameSize, std::vector<std::string> &out);
    void generateEpilogue(int frameSize, std::vector<std::string> &out);

    void saveCallerSaved(std::vector<std::string> &out);
    void restoreCallerSaved(std::vector<std::string> &out);


    // -----------------------------
    // IR → Assembly
    // -----------------------------
    void translate(const IRInstruction &inst, std::vector<std::string> &outAsm);


    // -----------------------------
    // Helpers
    // -----------------------------
    std::string regName(int id) const;                     // deprecated
    std::string labelName(const std::string &name) const;
    bool isMIPSRegister(const std::string &name) const;
    void initRegisterPools();                              // initialize pools
};

#endif // CODEGEN_HPP
