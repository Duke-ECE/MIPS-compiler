/**
 * @file reg_allocator.hpp
 * @brief 寄存器分配器，支持活跃性分析和寄存器溢出
 */

#ifndef REG_ALLOCATOR_HPP
#define REG_ALLOCATOR_HPP

#include "compiler/ir.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

/**
 * @brief 变量的存储位置信息
 */
struct VarLocation {
    bool inReg;              // 是否在寄存器中
    std::string reg;         // 寄存器名称（如 $t0, $s0）
    int stackOffset;         // 栈偏移量（如果被溢出）
    
    VarLocation() : inReg(false), stackOffset(0) {}
};

/**
 * @brief 变量的活跃区间
 */
struct LiveInterval {
    std::string varName;
    int start;               // 开始指令索引
    int end;                 // 结束指令索引
    
    LiveInterval(const std::string &name, int s, int e) 
        : varName(name), start(s), end(e) {}
};

/**
 * @brief 寄存器分配器类
 */
class RegisterAllocator {
public:
    RegisterAllocator();
    
    /**
     * @brief 对IR程序进行寄存器分配
     * @param program 输入的IR程序
     * @return 插入了spill/reload指令的IR程序
     */
    IRProgram allocate(const IRProgram &program);
    
    /**
     * @brief 获取变量的当前位置
     */
    const VarLocation& getLocation(const std::string &var) const;
    
    /**
     * @brief 检查变量是否已分配
     */
    bool hasLocation(const std::string &var) const;
    
    /**
     * @brief 获取下一个spill偏移量
     */
    int getNextSpillOffset() const { return nextSpillOffset; }
    
    /**
     * @brief 获取使用的callee-saved寄存器
     */
    const std::unordered_map<std::string, bool>& getUsedCalleeSaved() const {
        return usedCalleeSaved;
    }
    
private:
    // ========================================
    // 数据结构
    // ========================================
    std::unordered_map<std::string, VarLocation> varLocations;  // 变量位置表
    std::vector<std::string> freeRegs;                          // 可用寄存器池
    std::unordered_set<std::string> usedRegs;                   // 已使用的寄存器
    int nextSpillOffset;                                        // 下一个spill栈偏移
    
    // 活跃性分析结果
    std::vector<std::unordered_set<std::string>> liveIn;        // 每条指令的live-in
    std::vector<std::unordered_set<std::string>> liveOut;       // 每条指令的live-out
    
    // 函数级别的状态
    std::unordered_map<std::string, bool> usedCalleeSaved;      // 使用的callee-saved寄存器
    int currentFunctionStart;                                    // 当前函数起始位置
    
    // ========================================
    // 活跃性分析
    // ========================================
    
    /**
     * @brief 执行活跃性分析
     */
    void performLivenessAnalysis(const std::vector<IRInstruction> &instructions);
    
    /**
     * @brief 获取指令使用的变量
     */
    std::unordered_set<std::string> getUsedVars(const IRInstruction &inst) const;
    
    /**
     * @brief 获取指令定义的变量
     */
    std::unordered_set<std::string> getDefinedVars(const IRInstruction &inst) const;
    
    /**
     * @brief 检查字符串是否为变量（而非寄存器或立即数）
     */
    bool isVariable(const std::string &name) const;
    
    // ========================================
    // 活跃区间构建
    // ========================================
    
    /**
     * @brief 构建所有变量的活跃区间
     */
    std::vector<LiveInterval> buildLiveIntervals(
        const std::vector<IRInstruction> &instructions) const;
    
    // ========================================
    // 线性扫描寄存器分配
    // ========================================
    
    /**
     * @brief 使用线性扫描算法分配寄存器
     */
    void linearScanAllocation(const std::vector<LiveInterval> &intervals,
                             const std::vector<IRInstruction> &instructions);
    
    /**
     * @brief 分配寄存器给变量
     */
    std::string allocateRegister(const std::string &var);
    
    /**
     * @brief 溢出一个变量到栈
     */
    std::string spillVariable(const std::string &var);
    
    /**
     * @brief 释放寄存器
     */
    void freeRegister(const std::string &reg);
    
    /**
     * @brief 选择要溢出的受害者变量
     */
    std::string selectSpillVictim(int currentIdx) const;
    
    // ========================================
    // Spill/Reload 指令插入
    // ========================================
    
    /**
     * @brief 插入spill和reload指令
     */
    IRProgram insertSpillReload(const IRProgram &program);
    
    /**
     * @brief 为指令生成必要的load指令
     */
    void generateLoads(const IRInstruction &inst, 
                      std::vector<IRInstruction> &output,
                      int instIdx);
    
    /**
     * @brief 为指令生成必要的store指令
     */
    void generateStores(const IRInstruction &inst,
                       std::vector<IRInstruction> &output,
                       int instIdx);
    
    // ========================================
    // 工具函数
    // ========================================
    
    /**
     * @brief 初始化寄存器池
     */
    void initRegisterPools();
    
    /**
     * @brief 重置分配器状态（用于新函数）
     */
    void resetForNewFunction();
};

#endif // REG_ALLOCATOR_HPP
