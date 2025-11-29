#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "compiler/ir.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Optimizer {
public:
    Optimizer() = default;

    // 对 IR 程序做优化（多轮直到无变化）
    IRProgram optimize(const IRProgram &prog);

private:
    // ===== 3 个优化 Pass =====
    bool propagateConstants(std::vector<IRInstruction> &insts);
    bool propagateCopies(std::vector<IRInstruction> &insts);
    bool eliminateDeadTemps(std::vector<IRInstruction> &insts);

    // ===== 工具函数 =====
    bool isTemp(const std::string &name) const;
    bool isNumber(const std::string &s) const;
};

#endif // OPTIMIZER_HPP
