/**
 * @file asm_parser.hpp
 * @brief 汇编器语法分析器
 * 
 * AsmParser 负责将 Token 序列解析为汇编指令表示。
 * 
 * 功能：
 * - 第一遍扫描：收集标签，建立符号表（标签 → 地址）
 * - 第二遍扫描：解析指令，生成指令列表
 * - 分支偏移计算
 * - 语法错误检测和报告
 * 
 * 支持的指令格式：
 * - R-type: add $rd, $rs, $rt
 * - I-type: addi $rd, $rs, imm / lw $rd, offset($rs) / beq $rd, $rs, label
 * - J-type: j label / jal label
 * - I/O:    input $rd / output $rd
 * - JR:     jr $rd
 * 
 * 示例：
 *   main:
 *       addi $r1, $r0, 10
 *       addi $r2, $r0, 20
 *       add $r3, $r1, $r2
 *       beq $r3, $r0, end
 *       j main
 *   end:
 *       output $r3
 */

#ifndef ASSEMBLER_ASM_PARSER_HPP
#define ASSEMBLER_ASM_PARSER_HPP

#include "asm_lexer.hpp"
#include "asm_token.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace assembler {

// ============================================================================
// 错误处理
// ============================================================================

/**
 * @brief 汇编错误类型
 */
enum class AsmErrorType {
    SYNTAX_ERROR,           // 语法错误
    UNDEFINED_LABEL,        // 未定义的标签
    DUPLICATE_LABEL,        // 重复定义的标签
    INVALID_REGISTER,       // 无效的寄存器
    INVALID_INSTRUCTION,    // 无效的指令
    INVALID_OPERAND,        // 无效的操作数
    IMMEDIATE_OUT_OF_RANGE, // 立即数超出范围
    TARGET_OUT_OF_RANGE,    // 跳转目标超出范围
    UNEXPECTED_TOKEN,       // 意外的 Token
    MISSING_OPERAND,        // 缺少操作数
    EXTRA_OPERAND,          // 多余的操作数
};

/**
 * @brief 汇编错误信息
 */
struct AsmError {
    AsmErrorType type;      // 错误类型
    std::string message;    // 错误消息
    int line;               // 行号
    int column;             // 列号
    std::string context;    // 上下文（相关的源代码片段）
    
    AsmError(AsmErrorType t, const std::string& msg, int ln, int col, const std::string& ctx = "")
        : type(t), message(msg), line(ln), column(col), context(ctx) {}
};

/**
 * @brief 错误类型转换为字符串
 */
std::string asmErrorTypeToString(AsmErrorType type);

// ============================================================================
// 解析后的指令表示
// ============================================================================

/**
 * @brief 解析后的汇编指令
 * 
 * 存储解析后的指令信息，包括原始文本、操作码、操作数等。
 * 在第一遍扫描时，标签引用可能还未解析，用 labelRef 存储。
 * 在第二遍扫描后，所有地址都被解析为数值。
 */
struct ParsedInstruction {
    // 源信息
    int line;                   // 源代码行号
    uint32_t address;           // 指令地址（字地址）
    
    // 指令信息
    isa::Opcode opcode;         // 操作码
    isa::InstructionType type;  // 指令类型（R/I/J）
    
    // 操作数
    isa::Register rd;           // 目标寄存器
    isa::Register rs;           // 源寄存器1
    isa::Register rt;           // 源寄存器2（R-type）
    int32_t immediate;          // 立即数（I-type）
    uint32_t target;            // 跳转目标（J-type）
    
    // 标签引用（在解析分支/跳转时使用）
    std::string labelRef;       // 如果引用标签，存储标签名
    bool hasLabelRef;           // 是否有未解析的标签引用
    
    // 原始文本（用于调试/反汇编）
    std::string originalText;
    
    /**
     * @brief 默认构造函数
     */
    ParsedInstruction()
        : line(0), address(0), opcode(isa::Opcode::INVALID), type(isa::InstructionType::INVALID),
          rd(isa::Register::R0), rs(isa::Register::R0), rt(isa::Register::R0),
          immediate(0), target(0), labelRef(""), hasLabelRef(false), originalText("") {}
};

// ============================================================================
// 符号表
// ============================================================================

/**
 * @brief 符号信息
 */
struct SymbolInfo {
    std::string name;       // 符号名（标签名）
    uint32_t address;       // 地址（字地址）
    int definedLine;        // 定义所在行
    bool isResolved;        // 是否已解析
    
    SymbolInfo() : name(""), address(0), definedLine(0), isResolved(false) {}
    SymbolInfo(const std::string& n, uint32_t addr, int line)
        : name(n), address(addr), definedLine(line), isResolved(true) {}
};

/**
 * @brief 符号表类型（标签名 → 符号信息）
 */
using SymbolTable = std::map<std::string, SymbolInfo>;

// ============================================================================
// 解析结果
// ============================================================================

/**
 * @brief 汇编解析结果
 */
struct ParseResult {
    bool success;                               // 解析是否成功
    std::vector<ParsedInstruction> instructions; // 解析后的指令列表
    SymbolTable symbols;                        // 符号表
    std::vector<AsmError> errors;               // 错误列表
    std::vector<std::string> warnings;          // 警告列表
    
    // 段信息
    uint32_t textStart;     // 代码段起始地址
    uint32_t textSize;      // 代码段大小（字数）
    uint32_t dataStart;     // 数据段起始地址
    uint32_t dataSize;      // 数据段大小（字数）
    
    ParseResult() 
        : success(false), textStart(0), textSize(0), dataStart(0), dataSize(0) {}
};

// ============================================================================
// AsmParser 类
// ============================================================================

/**
 * @brief 汇编器语法分析器类
 */
class AsmParser {
public:
    /**
     * @brief 构造函数
     * @param source 汇编源代码字符串
     */
    explicit AsmParser(const std::string& source);

    /**
     * @brief 从 Lexer 构造
     * @param lexer 已经初始化的 Lexer
     */
    explicit AsmParser(AsmLexer&& lexer);

    /**
     * @brief 解析汇编代码
     * @return 解析结果
     */
    ParseResult parse();

    /**
     * @brief 获取符号表
     * @return 符号表引用
     */
    const SymbolTable& getSymbolTable() const { return symbolTable; }

    /**
     * @brief 获取错误列表
     * @return 错误列表引用
     */
    const std::vector<AsmError>& getErrors() const { return errors; }

    /**
     * @brief 是否有错误
     * @return 如果有错误返回 true
     */
    bool hasErrors() const { return !errors.empty(); }

    /**
     * @brief 设置代码段起始地址
     * @param addr 起始地址
     */
    void setTextStart(uint32_t addr) { textStartAddress = addr; }

    /**
     * @brief 设置数据段起始地址
     * @param addr 起始地址
     */
    void setDataStart(uint32_t addr) { dataStartAddress = addr; }

private:
    // ==================== 解析阶段 ====================
    
    /**
     * @brief 第一遍扫描：收集标签
     */
    void firstPass();
    
    /**
     * @brief 第二遍扫描：解析指令
     */
    void secondPass();
    
    /**
     * @brief 解析标签引用，计算分支偏移
     */
    void resolveLabels();

    // ==================== 指令解析 ====================
    
    /**
     * @brief 解析一条指令
     * @return 解析后的指令，如果失败返回 nullopt
     */
    std::optional<ParsedInstruction> parseInstruction();
    
    /**
     * @brief 解析 R-type 指令
     * @param opcode 操作码
     * @return 解析后的指令
     */
    std::optional<ParsedInstruction> parseRTypeInstruction(isa::Opcode opcode);
    
    /**
     * @brief 解析 I-type 指令
     * @param opcode 操作码
     * @return 解析后的指令
     */
    std::optional<ParsedInstruction> parseITypeInstruction(isa::Opcode opcode);
    
    /**
     * @brief 解析 J-type 指令
     * @param opcode 操作码
     * @return 解析后的指令
     */
    std::optional<ParsedInstruction> parseJTypeInstruction(isa::Opcode opcode);
    
    /**
     * @brief 解析 load/store 指令的内存操作数 offset($rs)
     * @param offset 输出：偏移量
     * @param rs 输出：基址寄存器
     * @return 解析是否成功
     */
    bool parseMemoryOperand(int32_t& offset, isa::Register& rs);

    // ==================== Token 操作 ====================
    
    /**
     * @brief 获取当前 Token
     */
    AsmToken currentToken() const;
    
    /**
     * @brief 查看下一个 Token
     */
    AsmToken peekToken() const;
    
    /**
     * @brief 前进到下一个 Token
     */
    void advance();
    
    /**
     * @brief 消耗指定类型的 Token
     * @param type 期望的 Token 类型
     * @param errorMsg 错误消息
     * @return 消耗的 Token，如果类型不匹配返回无效 Token
     */
    AsmToken consume(AsmTokenType type, const std::string& errorMsg);
    
    /**
     * @brief 检查当前 Token 类型
     */
    bool check(AsmTokenType type) const;
    
    /**
     * @brief 匹配并消耗指定类型的 Token
     */
    bool match(AsmTokenType type);
    
    /**
     * @brief 跳过换行符
     */
    void skipNewlines();
    
    /**
     * @brief 跳到下一行（错误恢复用）
     */
    void synchronize();
    
    /**
     * @brief 是否到达结尾
     */
    bool isAtEnd() const;

    // ==================== 辅助函数 ====================
    
    /**
     * @brief 解析寄存器
     * @return 寄存器，如果失败返回 INVALID
     */
    isa::Register parseRegister();
    
    /**
     * @brief 解析立即数或标签
     * @param value 输出：解析后的值
     * @param labelRef 输出：标签引用（如果是标签）
     * @return 解析是否成功
     */
    bool parseImmediateOrLabel(int32_t& value, std::string& labelRef);
    
    /**
     * @brief 查找标签地址
     * @param label 标签名
     * @return 地址，如果未找到返回 nullopt
     */
    std::optional<uint32_t> lookupLabel(const std::string& label) const;
    
    /**
     * @brief 计算分支偏移
     * @param currentAddr 当前指令地址
     * @param targetAddr 目标地址
     * @return 偏移量
     */
    int32_t calculateBranchOffset(uint32_t currentAddr, uint32_t targetAddr) const;
    
    /**
     * @brief 检查立即数范围
     */
    bool isImmediateInRange(int32_t value) const;
    
    /**
     * @brief 检查跳转目标范围
     */
    bool isTargetInRange(uint32_t target) const;

    // ==================== 错误处理 ====================
    
    /**
     * @brief 报告错误
     */
    void reportError(AsmErrorType type, const std::string& message);
    void reportError(AsmErrorType type, const std::string& message, int line, int column);
    
    /**
     * @brief 报告警告
     */
    void reportWarning(const std::string& message);

private:
    AsmLexer lexer;                         // 词法分析器
    std::vector<AsmToken> tokens;           // Token 列表
    size_t currentIndex;                    // 当前 Token 索引
    
    SymbolTable symbolTable;                // 符号表
    std::vector<ParsedInstruction> instructions; // 指令列表
    std::vector<AsmError> errors;           // 错误列表
    std::vector<std::string> warnings;      // 警告列表
    
    uint32_t currentAddress;                // 当前地址（字地址）
    uint32_t textStartAddress;              // 代码段起始地址
    uint32_t dataStartAddress;              // 数据段起始地址
    
    bool inTextSection;                     // 当前是否在代码段
    bool inDataSection;                     // 当前是否在数据段
};

} // namespace assembler

#endif // ASSEMBLER_ASM_PARSER_HPP
