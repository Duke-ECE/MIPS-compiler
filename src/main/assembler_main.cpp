/**
 * @file assembler_main.cpp
 * @brief 汇编器模块入口
 * 
 * 实现 Assembler 类，提供汇编器的顶层接口。
 * 该模块负责将汇编源代码转换为机器码，并支持输出为 HEX/MIF 格式。
 * 
 * 注意：此文件不包含 main() 函数，仅作为汇编器功能模块。
 * main() 函数位于 src/main.cpp 中。
 */

#include "assembler/assembler.hpp"
#include "assembler/asm_parser.hpp"
#include "assembler/asm_encoder.hpp"
#include "utils/file_io.hpp"
#include <stdexcept>
#include <sstream>

namespace assembler {

// ============================================================================
// Assembler 类实现
// ============================================================================

std::vector<uint32_t> Assembler::assemble(const std::string& asmText) {
    AssembleResult result = assembleWithResult(asmText);
    
    if (!result.success) {
        // 构建错误消息
        std::ostringstream oss;
        oss << "汇编失败:\n";
        for (const auto& error : result.errors) {
            oss << "  " << error << "\n";
        }
        throw std::runtime_error(oss.str());
    }
    
    return result.code;
}

AssembleResult Assembler::assembleWithResult(const std::string& asmText) {
    AssembleResult result;
    lastErrors.clear();
    lastWarnings.clear();
    
    // 1. 语法分析（包含词法分析）
    AsmParser parser(asmText);
    ParseResult parseResult = parser.parse();
    
    if (!parseResult.success) {
        result.success = false;
        for (const auto& error : parseResult.errors) {
            std::ostringstream oss;
            oss << "行 " << error.line << ", 列 " << error.column << ": "
                << asmErrorTypeToString(error.type) << " - " << error.message;
            result.errors.push_back(oss.str());
        }
        lastErrors = result.errors;
        return result;
    }
    
    // 复制警告
    result.warnings = parseResult.warnings;
    lastWarnings = result.warnings;
    
    // 2. 编码生成
    AsmEncoder encoder;
    EncodeResult encodeResult = encoder.encode(parseResult);
    
    if (!encodeResult.success) {
        result.success = false;
        for (const auto& error : encodeResult.errors) {
            std::ostringstream oss;
            oss << "行 " << error.line << ": "
                << encoderErrorTypeToString(error.type) << " - " << error.message;
            result.errors.push_back(oss.str());
        }
        lastErrors = result.errors;
        return result;
    }
    
    // 3. 提取指令内存（代码段 -> imem）
    result.success = true;
    result.imem.reserve(encodeResult.instructions.size());
    for (const auto& instr : encodeResult.instructions) {
        result.imem.push_back(instr.machineCode);
    }
    
    // 4. 提取数据内存（数据段 -> dmem）
    result.dmem.reserve(parseResult.dataWords.size());
    for (const auto& dataWord : parseResult.dataWords) {
        result.dmem.push_back(dataWord.value);
    }
    
    // 5. code 保持兼容（仅包含 imem）
    result.code = result.imem;
    
    return result;
}

void Assembler::writeMIF(const std::vector<uint32_t>& code, const std::string& path) {
    utils::FileResult fileResult = utils::FileIO::writeMif(path, code);
    if (!fileResult.success) {
        throw std::runtime_error("写入 MIF 文件失败: " + fileResult.error);
    }
}

void Assembler::writeHEX(const std::vector<uint32_t>& code, const std::string& path) {
    utils::FileResult fileResult = utils::FileIO::writeHex(path, code);
    if (!fileResult.success) {
        throw std::runtime_error("写入 HEX 文件失败: " + fileResult.error);
    }
}

} // namespace assembler
