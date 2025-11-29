#include "compiler/optimizer.hpp"
#include <cctype>
#include <algorithm>
#include <iostream>

using std::string;
using std::vector;

// ------------------------------------------------------------
// 工具函数
// ------------------------------------------------------------
bool Optimizer::isTemp(const string &name) const {
    return !name.empty() && name[0] == 't' && isdigit(name[1]);
}

bool Optimizer::isNumber(const string &s) const {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-') i++;
    for (; i < s.size(); ++i) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

// ------------------------------------------------------------
// 总控 optimize()
// 多次迭代直到所有 pass 都没有改变 IR
// ------------------------------------------------------------
IRProgram Optimizer::optimize(const IRProgram &prog) {
    IRProgram result = prog;
    bool changed = true;

    while (changed) {
        changed = false;
        // 暂时禁用常量传播，因为它在某些情况下会产生错误的替换
        // TODO: 修复常量传播的bug后重新启用
        // changed |= propagateConstants(result.instructions);
        changed |= eliminateLoadStore(result.instructions);  // Load-Store消除
        changed |= propagateCopies(result.instructions);     // 复制传播
        changed |= eliminateDeadTemps(result.instructions);  // 死代码消除
    }

    return result;
}

// ------------------------------------------------------------
// Pass 1: 常量传播
// t0 = 3
// x = t0     → x = 3
// ------------------------------------------------------------
bool Optimizer::propagateConstants(vector<IRInstruction> &insts) {
    bool changed = false;
    std::unordered_map<string, string> constValue;

    // 收集简单常量 mov
    for (auto &ins : insts) {
        if (ins.op == "mov" && isNumber(ins.src1)) {
            constValue[ins.dst] = ins.src1;
        }
    }

    // 传播
    for (auto &ins : insts) {
        auto sub = [&](string &operand) {
            if (constValue.count(operand)) {
                operand = constValue[operand];
                changed = true;
            }
        };

        sub(ins.src1);
        sub(ins.src2);
    }

    return changed;
}

// ------------------------------------------------------------
// Pass 2: Copy Propagation
// t1 = x
// y  = t1 + 5    → y = x + 5
// ------------------------------------------------------------
bool Optimizer::propagateCopies(vector<IRInstruction> &insts) {
    bool changed = false;
    std::unordered_map<string, string> copyOf;

    // 收集纯复制 mov
    for (auto &ins : insts) {
        if (ins.op == "mov" && !isNumber(ins.src1)) {
            copyOf[ins.dst] = ins.src1;
        }
    }

    // 传播复制
    for (auto &ins : insts) {
        auto sub = [&](string &operand) {
            if (copyOf.count(operand)) {
                operand = copyOf[operand];
                changed = true;
            }
        };

        sub(ins.src1);
        sub(ins.src2);
    }

    return changed;
}

// ------------------------------------------------------------
// Pass 3: Load-Store Elimination
// 识别并消除冗余的 store-load 序列
// IR格式：
//   store src, offset  (将src存储到fp+offset)
//   load  dst, offset  (从fp+offset加载到dst)
// 优化为：
//   store src, offset  (保留store，可能其他地方需要)
//   mov   dst, src     (直接复制，避免内存访问)
// ------------------------------------------------------------
bool Optimizer::eliminateLoadStore(vector<IRInstruction> &insts) {
    bool changed = false;
    vector<IRInstruction> newInsts;
    newInsts.reserve(insts.size());
    
    for (size_t i = 0; i < insts.size(); i++) {
        // 检查是否是 store 指令
        if (insts[i].op == "store" && i + 1 < insts.size()) {
            auto &storeIns = insts[i];
            auto &nextIns = insts[i + 1];
            
            // 检查下一条是否是 load，且偏移量（地址）相同
            // store: dst=要存储的值, src1=偏移量
            // load:  dst=目标寄存器, src1=偏移量
            if (nextIns.op == "load" && storeIns.src1 == nextIns.src1) {
                // 找到 store-load 序列
                newInsts.push_back(storeIns);  // 保留 store
                
                // 将 load 替换为 mov：直接从 store 的源复制
                IRInstruction movIns;
                movIns.op = "mov";
                movIns.dst = nextIns.dst;      // load 的目标
                movIns.src1 = storeIns.dst;    // store 的源（要存储的值）
                movIns.src2 = "";
                newInsts.push_back(movIns);
                
                i++;  // 跳过已处理的 load 指令
                changed = true;
                continue;
            }
        }
        
        newInsts.push_back(insts[i]);
    }
    
    insts.swap(newInsts);
    return changed;
}

// ------------------------------------------------------------
// Pass 4: Dead Temp Elimination  
// 极保守策略：只删除临时变量到临时变量的纯mov操作
// 保留所有load、store、算术运算等可能影响最终代码的指令
// ------------------------------------------------------------
bool Optimizer::eliminateDeadTemps(vector<IRInstruction> &insts) {
    bool changed = false;

    // 找出所有被使用的临时变量
    std::unordered_set<string> usedTemps;

    for (auto &ins : insts) {
        if (isTemp(ins.src1)) usedTemps.insert(ins.src1);
        if (isTemp(ins.src2)) usedTemps.insert(ins.src2);
    }

    vector<IRInstruction> newInsts;
    newInsts.reserve(insts.size());

    for (auto &ins : insts) {
        // 极保守策略：只删除满足ALL以下条件的指令：
        // 1. 操作是 mov（纯复制）
        // 2. 目标和源都是临时变量（临时变量到临时变量）
        // 3. 目标临时变量未被使用
        // 保留所有其他指令，包括：
        // - 常量加载 (mov t0, 3)
        // - 算术运算 (add, sub, mul等)
        // - load/store操作
        // - 任何涉及非临时变量的操作
        if (ins.op == "mov" && 
            isTemp(ins.dst) && 
            isTemp(ins.src1) &&
            usedTemps.count(ins.dst) == 0) {
            // 这是一个未使用的临时变量复制操作，可以安全删除
            changed = true;
            continue;
        }
        
        newInsts.push_back(ins);
    }

    insts.swap(newInsts);
    return changed;
}
