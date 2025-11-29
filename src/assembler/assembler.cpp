/**
 * @file assembler.cpp
 * @brief Assembler 类实现
 * 
 * 实现汇编器的核心流水线：
 * - Lexer 阶段：源代码 → Token 序列
 * - Parser 阶段：Token 序列 → 解析结果（指令列表 + 符号表）
 * - Encoder 阶段：解析结果 → 机器码
 */

#include "assembler/assembler.hpp"
#include "assembler/asm_lexer.hpp"
#include "assembler/asm_parser.hpp"
#include "assembler/asm_encoder.hpp"
#include <sstream>
#include <stdexcept>

namespace assembler {

/*****************************************
 * 1. Lexer 阶段
 *****************************************/
std::vector<AsmToken> Assembler::runLexer(const std::string& asmText) {
    try {
        AsmLexer lexer(asmText);
        return lexer.tokenize(true, false);  // 包含换行，不包含注释
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Lexer error: ") + e.what());
    }
}

/*****************************************
 * 2. Parser 阶段
 *****************************************/
ParseResult Assembler::runParser(const std::string& asmText) {
    try {
        AsmParser parser(asmText);
        return parser.parse();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Parser error: ") + e.what());
    }
}

/*****************************************
 * 3. Encoder 阶段
 *****************************************/
EncodeResult Assembler::runEncoder(const ParseResult& parseResult) {
    try {
        AsmEncoder encoder;
        return encoder.encode(parseResult);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Encoder error: ") + e.what());
    }
}

/*****************************************
 * 4. 顶层接口：Assembly Text → Machine Code
 *****************************************/
AssembleResult Assembler::assemble(const std::string& asmText) {
    AssembleResult result;
    
    try {
        // ---------- Parser 阶段（内含 Lexer）----------
        ParseResult parseResult = runParser(asmText);
        
        if (!parseResult.success) {
            result.success = false;
            for (const auto& error : parseResult.errors) {
                std::ostringstream oss;
                oss << "行 " << error.line << ", 列 " << error.column << ": "
                    << asmErrorTypeToString(error.type) << " - " << error.message;
                result.errors.push_back(oss.str());
            }
            return result;
        }
        
        // 复制警告
        result.warnings = parseResult.warnings;
        
        // ---------- Encoder 阶段 ----------
        EncodeResult encodeResult = runEncoder(parseResult);
        
        if (!encodeResult.success) {
            result.success = false;
            for (const auto& error : encodeResult.errors) {
                std::ostringstream oss;
                oss << "行 " << error.line << ": "
                    << encoderErrorTypeToString(error.type) << " - " << error.message;
                result.errors.push_back(oss.str());
            }
            return result;
        }
        
        // ---------- 提取结果 ----------
        result.success = true;
        
        // 提取指令内存（代码段 -> imem）
        result.imem.reserve(encodeResult.instructions.size());
        for (const auto& instr : encodeResult.instructions) {
            result.imem.push_back(instr.machineCode);
        }
        
        // 提取数据内存（数据段 -> dmem）
        result.dmem.reserve(parseResult.dataWords.size());
        for (const auto& dataWord : parseResult.dataWords) {
            result.dmem.push_back(dataWord.value);
        }
        
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errors.push_back(std::string("Assembler failed: ") + e.what());
        return result;
    }
}

} // namespace assembler
