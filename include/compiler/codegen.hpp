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
    std::unordered_map<std::string, std::string> regMap;   // variable → register
    std::vector<std::string> availableTemps;               // $t0–$t9
    std::vector<std::string> availableSaved;               // $s0–$s7

    // NEW: fixed ordering for callee-saved registers
    std::vector<std::string> savedOrder;                   // { $s0, $s1, ..., $s7 }

    std::unordered_map<std::string, bool> usedCalleeSaved; // which $s registers used

    int currentFrameSize;                                  // stack frame size


    // -----------------------------
    // Register allocators
    // -----------------------------
    std::string allocateTemp();                            // get a $t register
    std::string allocateSaved();                           // get a $s register
    std::string allocateRegister(const std::string &var);  // var → register
    void releaseRegister(const std::string &reg);          // return register


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
