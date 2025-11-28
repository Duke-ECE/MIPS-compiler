/**
 * @file asm_encoder.hpp
 * @brief 汇编器编码器
 * 
 * AsmEncoder 负责将解析后的汇编指令编码为 32 位机器码。
 * 
 * 功能：
 * - 根据 ISA 格式编码 R-type、I-type、J-type 指令
 * - 处理符号引用和分支偏移
 * - 生成完整的机器码序列
 * - 支持多种输出格式（二进制、十六进制）
 * 
 * 指令格式（32位）：
 * - R-type: opcode(5) | rd(5) | rs(5) | rt(5) | zeros(12)
 * - I-type: opcode(5) | rd(5) | rs(5) | immediate(17, signed)
 * - J-type: opcode(5) | target(27)
 * 
 * 示例：
 *   add $r1, $r2, $r3  -> 0x00441000 (opcode=0, rd=1, rs=2, rt=3)
 *   addi $r1, $r0, 10  -> 0x3040000A (opcode=6, rd=1, rs=0, imm=10)
 *   j 100              -> 0x60000064 (opcode=12, target=100)
 */

#ifndef ASSEMBLER_ASM_ENCODER_HPP
#define ASSEMBLER_ASM_ENCODER_HPP

#include "asm_parser.hpp"
#include "isa/Opcode.hpp"
#include "isa/Registers.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace assembler {

// ============================================================================
// 编码结果
// ============================================================================

/**
 * @brief 单条指令的编码结果
 */
struct EncodedInstruction {
    uint32_t machineCode;       // 32位机器码
    uint32_t address;           // 指令地址（字地址）
    int sourceLine;             // 源代码行号
    std::string sourceText;     // 原始汇编文本
    std::string disassembly;    // 反汇编文本（用于验证）
    
    EncodedInstruction()
        : machineCode(0), address(0), sourceLine(0), sourceText(""), disassembly("") {}
    
    EncodedInstruction(uint32_t code, uint32_t addr, int line, 
                       const std::string& src = "", const std::string& dis = "")
        : machineCode(code), address(addr), sourceLine(line), 
          sourceText(src), disassembly(dis) {}
};

/**
 * @brief 编码错误类型
 */
enum class EncoderErrorType {
    INVALID_OPCODE,             // 无效的操作码
    INVALID_REGISTER,           // 无效的寄存器
    IMMEDIATE_OUT_OF_RANGE,     // 立即数超出范围
    TARGET_OUT_OF_RANGE,        // 跳转目标超出范围
    UNRESOLVED_LABEL,           // 未解析的标签
    ENCODING_FAILED,            // 编码失败
};

/**
 * @brief 编码错误信息
 */
struct EncoderError {
    EncoderErrorType type;      // 错误类型
    std::string message;        // 错误消息
    int line;                   // 源代码行号
    uint32_t address;           // 指令地址
    
    EncoderError(EncoderErrorType t, const std::string& msg, int ln = 0, uint32_t addr = 0)
        : type(t), message(msg), line(ln), address(addr) {}
};

/**
 * @brief 完整的编码结果
 */
struct EncodeResult {
    bool success;                                   // 编码是否成功
    std::vector<EncodedInstruction> instructions;   // 编码后的指令列表
    std::vector<EncoderError> errors;               // 错误列表
    std::vector<std::string> warnings;              // 警告列表
    
    // 统计信息
    uint32_t totalInstructions;     // 总指令数
    uint32_t textStartAddress;      // 代码段起始地址
    uint32_t textEndAddress;        // 代码段结束地址
    
    EncodeResult()
        : success(false), totalInstructions(0), textStartAddress(0), textEndAddress(0) {}
};

/**
 * @brief 错误类型转换为字符串
 */
std::string encoderErrorTypeToString(EncoderErrorType type);

// ============================================================================
// AsmEncoder 类
// ============================================================================

/**
 * @brief 汇编器编码器类
 */
class AsmEncoder {
public:
    /**
     * @brief 默认构造函数
     */
    AsmEncoder();

    /**
     * @brief 编码解析结果
     * @param parseResult 解析器的输出
     * @return 编码结果
     */
    EncodeResult encode(const ParseResult& parseResult);

    /**
     * @brief 编码单条解析后的指令
     * @param instr 解析后的指令
     * @param symbolTable 符号表（用于解析标签）
     * @return 编码后的机器码，如果失败返回 nullopt
     */
    std::optional<uint32_t> encodeInstruction(const ParsedInstruction& instr,
                                               const SymbolTable& symbolTable);

    /**
     * @brief 编码 R-type 指令
     * @param opcode 操作码
     * @param rd 目标寄存器
     * @param rs 源寄存器1
     * @param rt 源寄存器2
     * @return 32位机器码
     */
    uint32_t encodeRType(isa::Opcode opcode, isa::Register rd, 
                         isa::Register rs, isa::Register rt);

    /**
     * @brief 编码 I-type 指令
     * @param opcode 操作码
     * @param rd 目标/源寄存器
     * @param rs 基址/比较寄存器
     * @param immediate 17位有符号立即数
     * @return 32位机器码
     */
    uint32_t encodeIType(isa::Opcode opcode, isa::Register rd, 
                         isa::Register rs, int32_t immediate);

    /**
     * @brief 编码 J-type 指令
     * @param opcode 操作码
     * @param target 27位目标地址
     * @return 32位机器码
     */
    uint32_t encodeJType(isa::Opcode opcode, uint32_t target);

    /**
     * @brief 获取错误列表
     */
    const std::vector<EncoderError>& getErrors() const { return errors; }

    /**
     * @brief 是否有错误
     */
    bool hasErrors() const { return !errors.empty(); }

    /**
     * @brief 清除错误
     */
    void clearErrors() { errors.clear(); }

    // ==================== 输出格式转换 ====================

    /**
     * @brief 将机器码转换为二进制字符串
     * @param code 32位机器码
     * @return 32位二进制字符串（带空格分隔）
     */
    static std::string toBinaryString(uint32_t code);

    /**
     * @brief 将机器码转换为十六进制字符串
     * @param code 32位机器码
     * @param prefix 是否添加 "0x" 前缀
     * @return 十六进制字符串
     */
    static std::string toHexString(uint32_t code, bool prefix = true);

    /**
     * @brief 生成机器码的详细分解
     * @param code 32位机器码
     * @return 各字段的详细分解字符串
     */
    static std::string disassembleFields(uint32_t code);

    // ==================== Intel HEX 文件输出 ====================

    /**
     * @brief 将机器码写入 Intel HEX 格式文件
     * 
     * Intel HEX 格式说明：
     * :LLAAAATT[DD...]CC
     * - LL: 数据字节数
     * - AAAA: 16位地址
     * - TT: 记录类型 (00=数据, 01=EOF)
     * - DD: 数据字节
     * - CC: 校验和（二补数，使整行字节和为0）
     * 
     * @param words 32位机器码序列
     * @param path 输出文件路径
     * @param startAddress 起始地址（字地址，默认0）
     * @return 是否写入成功
     */
    static bool writeHex(const std::vector<uint32_t>& words, const std::string& path,
                         uint32_t startAddress = 0);

    /**
     * @brief 将编码结果写入 Intel HEX 格式文件
     * @param result 编码结果
     * @param path 输出文件路径
     * @return 是否写入成功
     */
    static bool writeHex(const EncodeResult& result, const std::string& path);

    /**
     * @brief 生成 Intel HEX 格式字符串（不写入文件）
     * @param words 32位机器码序列
     * @param startAddress 起始地址（字地址，默认0）
     * @return Intel HEX 格式的完整字符串
     */
    static std::string generateHexString(const std::vector<uint32_t>& words,
                                          uint32_t startAddress = 0);

    /**
     * @brief 生成单条 Intel HEX 记录
     * @param type 记录类型 (0x00=数据, 0x01=EOF)
     * @param address 16位地址
     * @param data 数据字节
     * @return HEX 记录字符串（包含起始冒号）
     */
    static std::string generateHexRecord(uint8_t type, uint16_t address, 
                                          const std::vector<uint8_t>& data);

    /**
     * @brief 计算 Intel HEX 校验和
     * @param bytes 字节序列（不包含校验和本身）
     * @return 校验和字节
     */
    static uint8_t calculateHexChecksum(const std::vector<uint8_t>& bytes);

private:
    // ==================== 辅助函数 ====================

    /**
     * @brief 解析标签引用并获取地址
     * @param labelRef 标签名
     * @param symbolTable 符号表
     * @param address 输出：标签地址
     * @return 是否成功
     */
    bool resolveLabel(const std::string& labelRef, const SymbolTable& symbolTable,
                      uint32_t& address);

    /**
     * @brief 计算分支偏移
     * @param currentAddr 当前指令地址
     * @param targetAddr 目标地址
     * @return 分支偏移量
     */
    int32_t calculateBranchOffset(uint32_t currentAddr, uint32_t targetAddr);

    /**
     * @brief 验证立即数范围
     * @param immediate 立即数值
     * @return 是否在有效范围内
     */
    bool validateImmediate(int32_t immediate);

    /**
     * @brief 验证跳转目标范围
     * @param target 目标地址
     * @return 是否在有效范围内
     */
    bool validateTarget(uint32_t target);

    /**
     * @brief 报告错误
     */
    void reportError(EncoderErrorType type, const std::string& message,
                     int line = 0, uint32_t address = 0);

    /**
     * @brief 报告警告
     */
    void reportWarning(const std::string& message);

private:
    std::vector<EncoderError> errors;       // 错误列表
    std::vector<std::string> warnings;      // 警告列表
};

// ============================================================================
// 便捷编码函数
// ============================================================================

/**
 * @brief 快速编码 R-type 指令
 */
inline uint32_t quickEncodeRType(isa::Opcode opcode, isa::Register rd,
                                  isa::Register rs, isa::Register rt) {
    AsmEncoder encoder;
    return encoder.encodeRType(opcode, rd, rs, rt);
}

/**
 * @brief 快速编码 I-type 指令
 */
inline uint32_t quickEncodeIType(isa::Opcode opcode, isa::Register rd,
                                  isa::Register rs, int32_t immediate) {
    AsmEncoder encoder;
    return encoder.encodeIType(opcode, rd, rs, immediate);
}

/**
 * @brief 快速编码 J-type 指令
 */
inline uint32_t quickEncodeJType(isa::Opcode opcode, uint32_t target) {
    AsmEncoder encoder;
    return encoder.encodeJType(opcode, target);
}

} // namespace assembler

#endif // ASSEMBLER_ASM_ENCODER_HPP
