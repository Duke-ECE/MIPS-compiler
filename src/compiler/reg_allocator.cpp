#include "compiler/reg_allocator.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

RegisterAllocator::RegisterAllocator() 
    : nextSpillOffset(-3), currentFunctionStart(0) {
    initRegisterPools();
}

void RegisterAllocator::initRegisterPools() {
    // 初始化可用的临时寄存器和saved寄存器
    freeRegs = {
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"
    };
    usedRegs.clear();
    usedCalleeSaved.clear();
}

void RegisterAllocator::resetForNewFunction() {
    varLocations.clear();
    usedRegs.clear();
    usedCalleeSaved.clear();
    nextSpillOffset = -3;  // -1和-2保留给参数，从-3开始
    initRegisterPools();
}

// ========================================
// 主入口：寄存器分配
// ========================================

IRProgram RegisterAllocator::allocate(const IRProgram &program) {
    if (program.instructions.empty()) {
        return program;
    }
    
    // 简化版本：为每个变量按顺序分配寄存器
    // 处理所有函数，不在函数边界重置
    
    // 扫描所有指令，为遇到的每个变量分配寄存器
    for (const auto &inst : program.instructions) {
        // 处理函数边界 - 重置寄存器池但不清空已分配的映射
        if (inst.op == "label" && inst.dst.find("_func_") == 0) {
            // 重新初始化可用寄存器池
            initRegisterPools();
            usedRegs.clear();
            nextSpillOffset = -3;
        }
        
        // 为指令中的变量分配寄存器
        auto defs = getDefinedVars(inst);
        auto uses = getUsedVars(inst);
        
        // 先处理使用的变量
        for (const auto &var : uses) {
            if (!hasLocation(var)) {
                if (!freeRegs.empty()) {
                    std::string reg = allocateRegister(var);
                    VarLocation loc;
                    loc.inReg = true;
                    loc.reg = reg;
                    loc.stackOffset = 0;
                    varLocations[var] = loc;
                } else {
                    // 寄存器用完，使用栈
                    VarLocation loc;
                    loc.inReg = false;
                    loc.reg = "";
                    loc.stackOffset = nextSpillOffset;
                    nextSpillOffset--;
                    varLocations[var] = loc;
                }
            }
        }
        
        // 再处理定义的变量
        for (const auto &var : defs) {
            if (!hasLocation(var)) {
                if (!freeRegs.empty()) {
                    std::string reg = allocateRegister(var);
                    VarLocation loc;
                    loc.inReg = true;
                    loc.reg = reg;
                    loc.stackOffset = 0;
                    varLocations[var] = loc;
                } else {
                    // 寄存器用完，使用栈
                    VarLocation loc;
                    loc.inReg = false;
                    loc.reg = "";
                    loc.stackOffset = nextSpillOffset;
                    nextSpillOffset--;
                    varLocations[var] = loc;
                }
            }
        }
    }
    
    // 不插入spill/reload，让codegen处理
    return program;
}

// ========================================
// 活跃性分析
// ========================================

void RegisterAllocator::performLivenessAnalysis(
    const std::vector<IRInstruction> &instructions) {
    
    int n = instructions.size();
    liveIn.resize(n);
    liveOut.resize(n);
    
    // 初始化
    for (int i = 0; i < n; ++i) {
        liveIn[i].clear();
        liveOut[i].clear();
    }
    
    // 迭代直到不动点
    bool changed = true;
    int iterations = 0;
    const int MAX_ITERATIONS = 100;
    
    while (changed && iterations < MAX_ITERATIONS) {
        changed = false;
        iterations++;
        
        // 从后向前遍历
        for (int i = n - 1; i >= 0; --i) {
            const auto &inst = instructions[i];
            
            auto oldLiveIn = liveIn[i];
            
            // liveOut[i] = union of liveIn[successors]
            // 简化版本：liveOut[i] = liveIn[i+1]（除了跳转指令）
            if (i + 1 < n) {
                liveOut[i] = liveIn[i + 1];
            }
            
            // 处理跳转指令
            if (inst.op == "jmp" || inst.op == "beq" || inst.op == "bgt") {
                // 找到跳转目标
                std::string target = (inst.op == "jmp") ? inst.dst : inst.src2;
                for (int j = 0; j < n; ++j) {
                    if (instructions[j].op == "label" && 
                        instructions[j].dst == target) {
                        // 合并跳转目标的liveIn
                        for (const auto &var : liveIn[j]) {
                            liveOut[i].insert(var);
                        }
                        break;
                    }
                }
            }
            
            // liveIn[i] = use[i] ∪ (liveOut[i] - def[i])
            liveIn[i] = liveOut[i];
            
            // 移除被定义的变量
            auto defs = getDefinedVars(inst);
            for (const auto &def : defs) {
                liveIn[i].erase(def);
            }
            
            // 添加使用的变量
            auto uses = getUsedVars(inst);
            for (const auto &use : uses) {
                liveIn[i].insert(use);
            }
            
            // 检查是否改变
            if (liveIn[i] != oldLiveIn) {
                changed = true;
            }
        }
    }
}

std::unordered_set<std::string> RegisterAllocator::getUsedVars(
    const IRInstruction &inst) const {
    
    std::unordered_set<std::string> vars;
    
    // 检查src1
    if (!inst.src1.empty() && isVariable(inst.src1)) {
        vars.insert(inst.src1);
    }
    
    // 检查src2
    if (!inst.src2.empty() && isVariable(inst.src2)) {
        vars.insert(inst.src2);
    }
    
    // 特殊处理：某些指令dst也是source（如store）
    if (inst.op == "store" || inst.op == "param" || inst.op == "ret" ||
        inst.op == "output") {
        if (!inst.dst.empty() && isVariable(inst.dst)) {
            vars.insert(inst.dst);
        }
    }
    
    return vars;
}

std::unordered_set<std::string> RegisterAllocator::getDefinedVars(
    const IRInstruction &inst) const {
    
    std::unordered_set<std::string> vars;
    
    // 大多数指令定义dst
    if (inst.op != "label" && inst.op != "enter" && inst.op != "leave" &&
        inst.op != "jmp" && inst.op != "beq" && inst.op != "bgt" &&
        inst.op != "store" && inst.op != "param" && inst.op != "ret" &&
        inst.op != "output" && inst.op != "call") {
        
        if (!inst.dst.empty() && isVariable(inst.dst)) {
            vars.insert(inst.dst);
        }
    }
    
    // call指令定义dst
    if (inst.op == "call" && !inst.dst.empty() && isVariable(inst.dst)) {
        vars.insert(inst.dst);
    }
    
    // input指令定义dst
    if (inst.op == "input" && !inst.dst.empty() && isVariable(inst.dst)) {
        vars.insert(inst.dst);
    }
    
    return vars;
}

bool RegisterAllocator::isVariable(const std::string &name) const {
    if (name.empty()) return false;
    
    // 寄存器以$开头
    if (name[0] == '$') return false;
    
    // 立即数（纯数字或负数）
    if (name[0] == '-' || (name[0] >= '0' && name[0] <= '9')) {
        bool allDigits = true;
        for (size_t i = (name[0] == '-' ? 1 : 0); i < name.size(); ++i) {
            if (name[i] < '0' || name[i] > '9') {
                allDigits = false;
                break;
            }
        }
        if (allDigits) return false;
    }
    
    // 标签（通常以L开头或包含_func_）
    if (name[0] == 'L' || name.find("_func_") == 0) return false;
    
    return true;
}

// ========================================
// 活跃区间构建
// ========================================

std::vector<LiveInterval> RegisterAllocator::buildLiveIntervals(
    const std::vector<IRInstruction> &instructions) const {
    
    std::unordered_map<std::string, int> firstUse;
    std::unordered_map<std::string, int> lastUse;
    
    // 扫描所有指令，记录每个变量的第一次和最后一次使用
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto uses = getUsedVars(instructions[i]);
        auto defs = getDefinedVars(instructions[i]);
        
        // 记录使用
        for (const auto &var : uses) {
            if (firstUse.find(var) == firstUse.end()) {
                firstUse[var] = i;
            }
            lastUse[var] = i;
        }
        
        // 记录定义
        for (const auto &var : defs) {
            if (firstUse.find(var) == firstUse.end()) {
                firstUse[var] = i;
            }
            lastUse[var] = i;
        }
    }
    
    // 构建区间
    std::vector<LiveInterval> intervals;
    for (const auto &[var, start] : firstUse) {
        int end = lastUse[var];
        intervals.emplace_back(var, start, end);
    }
    
    // 按start排序
    std::sort(intervals.begin(), intervals.end(),
        [](const LiveInterval &a, const LiveInterval &b) {
            return a.start < b.start;
        });
    
    return intervals;
}

// ========================================
// 线性扫描寄存器分配
// ========================================

void RegisterAllocator::linearScanAllocation(
    const std::vector<LiveInterval> &intervals,
    const std::vector<IRInstruction> &instructions) {
    
    std::vector<LiveInterval> active;  // 当前活跃的区间
    
    for (const auto &interval : intervals) {
        // 释放所有已结束的区间
        active.erase(
            std::remove_if(active.begin(), active.end(),
                [&](const LiveInterval &a) {
                    if (a.end < interval.start) {
                        // 释放这个变量的寄存器
                        auto it = varLocations.find(a.varName);
                        if (it != varLocations.end() && it->second.inReg) {
                            freeRegister(it->second.reg);
                        }
                        return true;
                    }
                    return false;
                }),
            active.end()
        );
        
        // 为当前区间分配寄存器
        if (!freeRegs.empty()) {
            // 有空闲寄存器
            std::string reg = allocateRegister(interval.varName);
            VarLocation loc;
            loc.inReg = true;
            loc.reg = reg;
            loc.stackOffset = 0;
            varLocations[interval.varName] = loc;
            
            active.push_back(interval);
        } else {
            // 需要溢出
            // 简单策略：溢出end最大的变量
            auto spillIt = std::max_element(active.begin(), active.end(),
                [](const LiveInterval &a, const LiveInterval &b) {
                    return a.end < b.end;
                });
            
            if (spillIt != active.end() && spillIt->end > interval.end) {
                // 溢出找到的变量
                std::string spillVar = spillIt->varName;
                std::string reg = spillVariable(spillVar);
                
                // 为当前变量使用这个寄存器
                VarLocation loc;
                loc.inReg = true;
                loc.reg = reg;
                loc.stackOffset = 0;
                varLocations[interval.varName] = loc;
                
                // 更新active列表
                active.erase(spillIt);
                active.push_back(interval);
            } else {
                // 当前变量立即溢出（不分配寄存器，直接使用栈）
                VarLocation loc;
                loc.inReg = false;
                loc.reg = "";
                loc.stackOffset = nextSpillOffset;
                nextSpillOffset--;
                varLocations[interval.varName] = loc;
            }
        }
    }
}

std::string RegisterAllocator::allocateRegister(const std::string &var) {
    if (freeRegs.empty()) {
        throw std::runtime_error("No free registers available for " + var);
    }
    
    std::string reg = freeRegs.front();
    freeRegs.erase(freeRegs.begin());
    usedRegs.insert(reg);
    
    // 如果是saved寄存器，标记为已使用
    if (reg[1] == 's') {
        usedCalleeSaved[reg] = true;
    }
    
    return reg;
}

std::string RegisterAllocator::spillVariable(const std::string &var) {
    auto it = varLocations.find(var);
    if (it == varLocations.end() || !it->second.inReg) {
        throw std::runtime_error("Cannot spill variable not in register: " + var);
    }
    
    std::string reg = it->second.reg;
    int offset = nextSpillOffset;
    nextSpillOffset--;  // word-addressed，向下增长
    
    // 更新位置信息
    it->second.inReg = false;
    it->second.stackOffset = offset;
    
    // 寄存器保持分配状态，但返回给调用者重用
    return reg;
}

void RegisterAllocator::freeRegister(const std::string &reg) {
    if (usedRegs.find(reg) != usedRegs.end()) {
        usedRegs.erase(reg);
        freeRegs.push_back(reg);
    }
}

std::string RegisterAllocator::selectSpillVictim(int currentIdx) const {
    // 选择一个当前在寄存器中且end最大的变量
    std::string victim;
    int maxEnd = currentIdx;
    
    for (const auto &[var, loc] : varLocations) {
        if (loc.inReg) {
            // 简化：选择第一个找到的
            if (victim.empty()) {
                victim = var;
            }
        }
    }
    
    return victim;
}

// ========================================
// Spill/Reload 指令插入
// ========================================

IRProgram RegisterAllocator::insertSpillReload(const IRProgram &program) {
    IRProgram result;
    result.instructions.reserve(program.instructions.size() * 2);
    
    bool inFunction = false;
    
    for (size_t i = 0; i < program.instructions.size(); ++i) {
        const auto &inst = program.instructions[i];
        
        // 检测函数边界
        if (inst.op == "label" && inst.dst.find("_func_") == 0) {
            inFunction = true;
            resetForNewFunction();
        } else if (inst.op == "leave") {
            inFunction = false;
        }
        
        // 在函数内部处理spill/reload
        if (inFunction && inst.op != "label" && inst.op != "enter" && 
            inst.op != "leave") {
            
            // 为使用的变量生成reload
            generateLoads(inst, result.instructions, i);
        }
        
        // 添加原指令
        result.instructions.push_back(inst);
        
        // 为定义的变量生成spill（如果需要）
        if (inFunction && inst.op != "label" && inst.op != "enter" && 
            inst.op != "leave") {
            generateStores(inst, result.instructions, i);
        }
    }
    
    return result;
}

void RegisterAllocator::generateLoads(
    const IRInstruction &inst,
    std::vector<IRInstruction> &output,
    int instIdx) {
    
    auto uses = getUsedVars(inst);
    
    for (const auto &var : uses) {
        auto it = varLocations.find(var);
        if (it != varLocations.end() && !it->second.inReg) {
            // 需要reload
            // 分配一个临时寄存器
            if (!freeRegs.empty()) {
                std::string reg = allocateRegister(var);
                
                // 生成load指令
                IRInstruction loadInst;
                loadInst.op = "load";
                loadInst.dst = var;
                loadInst.src1 = std::to_string(it->second.stackOffset);
                output.push_back(loadInst);
                
                // 更新位置
                it->second.inReg = true;
                it->second.reg = reg;
            }
        }
    }
}

void RegisterAllocator::generateStores(
    const IRInstruction &inst,
    std::vector<IRInstruction> &output,
    int instIdx) {
    
    auto defs = getDefinedVars(inst);
    
    for (const auto &var : defs) {
        auto it = varLocations.find(var);
        if (it != varLocations.end() && !it->second.inReg) {
            // 变量被溢出，需要store
            IRInstruction storeInst;
            storeInst.op = "store";
            storeInst.dst = var;
            storeInst.src1 = std::to_string(it->second.stackOffset);
            output.push_back(storeInst);
        }
    }
}

// ========================================
// 查询接口
// ========================================

const VarLocation& RegisterAllocator::getLocation(const std::string &var) const {
    static VarLocation dummy;
    auto it = varLocations.find(var);
    if (it != varLocations.end()) {
        return it->second;
    }
    return dummy;
}

bool RegisterAllocator::hasLocation(const std::string &var) const {
    return varLocations.find(var) != varLocations.end();
}
