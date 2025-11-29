// /**
//  * @file assembler.cpp
//  * @brief 汇编器顶层接口实现
//  */

// #include "assembler/assembler.hpp"
// #include "assembler/asm_parser.hpp"
// #include "assembler/asm_encoder.hpp"
// #include "utils/file_io.hpp"
// #include <stdexcept>
// #include <sstream>

// namespace assembler {

// std::vector<uint32_t> Assembler::assemble(const std::string& asmText) {
//     AssembleResult result = assembleWithResult(asmText);
    
//     if (!result.success) {
//         // 构建错误消息
//         std::ostringstream oss;
//         oss << "汇编失败:\n";
//         for (const auto& error : result.errors) {
//             oss << "  " << error << "\n";
//         }
//         throw std::runtime_error(oss.str());
//     }
    
//     return result.code;
// }

// AssembleResult Assembler::assembleWithResult(const std::string& asmText) {
//     AssembleResult result;
//     lastErrors.clear();
//     lastWarnings.clear();
    
//     // 1. 语法分析（包含词法分析）
//     AsmParser parser(asmText);
//     ParseResult parseResult = parser.parse();
    
//     if (!parseResult.success) {
//         result.success = false;
//         for (const auto& error : parseResult.errors) {
//             std::ostringstream oss;
//             oss << "行 " << error.line << ", 列 " << error.column << ": "
//                 << asmErrorTypeToString(error.type) << " - " << error.message;
//             result.errors.push_back(oss.str());
//         }
//         lastErrors = result.errors;
//         return result;
//     }
    
//     // 复制警告
//     result.warnings = parseResult.warnings;
//     lastWarnings = result.warnings;
    
//     // 2. 编码生成
//     AsmEncoder encoder;
//     EncodeResult encodeResult = encoder.encode(parseResult);
    
//     if (!encodeResult.success) {
//         result.success = false;
//         for (const auto& error : encodeResult.errors) {
//             std::ostringstream oss;
//             oss << "行 " << error.line << ": "
//                 << encoderErrorTypeToString(error.type) << " - " << error.message;
//             result.errors.push_back(oss.str());
//         }
//         lastErrors = result.errors;
//         return result;
//     }
    
//     // 3. 提取机器码
//     result.success = true;
//     result.code.reserve(encodeResult.instructions.size());
//     for (const auto& instr : encodeResult.instructions) {
//         result.code.push_back(instr.machineCode);
//     }
    
//     return result;
// }

// void Assembler::writeMIF(const std::vector<uint32_t>& code, const std::string& path) {
//     utils::FileResult result = utils::FileIO::writeMif(path, code);
//     if (!result.success) {
//         throw std::runtime_error("写入 MIF 文件失败: " + result.error);
//     }
// }

// void Assembler::writeHEX(const std::vector<uint32_t>& code, const std::string& path) {
//     utils::FileResult result = utils::FileIO::writeHex(path, code);
//     if (!result.success) {
//         throw std::runtime_error("写入 HEX 文件失败: " + result.error);
//     }
// }

// } // namespace assembler
