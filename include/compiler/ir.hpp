/**
 * @file ir.hpp
 * @brief Intermediate Representation (IR) Definitions
 * 
 * This file defines the IR instruction structure and IR program container.
 * 
 * IR instruction structure:
 * - op: operation (e.g., add, sub, mov, lt, call, ret, jmp, label)
 * - dst: destination operand
 * - src1: source operand 1
 * - src2: source operand 2
 * 
 * IR program container:
 * - A collection of IR instructions.
 * 
 * Notes:
 * - This IR is a simple three-address code representation suitable for MIPS code generation.
 */
#ifndef IR_HPP
#define IR_HPP

#include <string>
#include <vector>

// 一条三地址指令
struct IRInstruction {
    std::string op;    // 操作符，如：add / sub / mov / lt / call / ret / jmp / label
    std::string dst;   // 目标
    std::string src1;  // 源1
    std::string src2;  // 源2
};

// 一个 IR 程序（函数 + 全局）
class IRProgram {
public:
    std::vector<IRInstruction> instructions;

    // 添加一条 IR 指令
    void emit(const IRInstruction &inst) {
        instructions.push_back(inst);
    }
};

#endif // IR_HPP
