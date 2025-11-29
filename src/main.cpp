// /**
//  * @file main.cpp
//  * @brief Duke 550 编译器工具链主程序
//  * 
//  * 使用方法:
//  *   ./main <input.s> [-o output] [-f format]
//  * 
//  * 参数:
//  *   input.s       汇编源文件
//  *   -o output     输出文件名（不含扩展名）
//  *   -f format     输出格式: hex, mif, both (默认 both)
//  * 
//  * 示例:
//  *   ./main test.s                    # 输出 test.hex 和 test.mif
//  *   ./main test.s -o output          # 输出 output.hex 和 output.mif
//  *   ./main test.s -f hex             # 只输出 test.hex
//  */

// #include "assembler/assembler.hpp"
// #include "utils/file_io.hpp"
// #include <iostream>
// #include <string>
// #include <iomanip>

// using namespace assembler;
// using namespace utils;

// /**
//  * @brief 打印使用帮助
//  */
// void printUsage(const char* programName) {
//     std::cout << "使用方法: " << programName << " <input.s> [-o output] [-f format]\n\n";
//     std::cout << "参数:\n";
//     std::cout << "  input.s       汇编源文件\n";
//     std::cout << "  -o output     输出文件名（不含扩展名）\n";
//     std::cout << "  -f format     输出格式: hex, mif, both (默认 both)\n\n";
//     std::cout << "示例:\n";
//     std::cout << "  " << programName << " test.s                # 输出 test.hex 和 test.mif\n";
//     std::cout << "  " << programName << " test.s -o output      # 输出 output.hex 和 output.mif\n";
//     std::cout << "  " << programName << " test.s -f hex         # 只输出 test.hex\n";
// }

// /**
//  * @brief 主函数
//  */
// int main(int argc, char* argv[]) {
//     // 检查参数
//     if (argc < 2) {
//         printUsage(argv[0]);
//         return 1;
//     }
    
//     // 解析命令行参数
//     std::string inputPath = argv[1];
//     std::string outputBase;
//     std::string format = "both";
    
//     for (int i = 2; i < argc; i++) {
//         std::string arg = argv[i];
//         if (arg == "-o" && i + 1 < argc) {
//             outputBase = argv[++i];
//         } else if (arg == "-f" && i + 1 < argc) {
//             format = argv[++i];
//         } else if (arg == "-h" || arg == "--help") {
//             printUsage(argv[0]);
//             return 0;
//         }
//     }
    
//     // 默认输出文件名
//     if (outputBase.empty()) {
//         outputBase = FileIO::getBaseName(inputPath);
//     }
    
//     std::cout << "============================================\n";
//     std::cout << "        Duke 550 汇编器\n";
//     std::cout << "============================================\n\n";
    
//     // 1. 读取源文件
//     std::cout << "[1/3] 读取源文件: " << inputPath << "\n";
    
//     auto sourceOpt = FileIO::readFile(inputPath);
//     if (!sourceOpt) {
//         std::cerr << "错误: 无法读取文件 '" << inputPath << "'\n";
//         return 1;
//     }
    
//     const std::string& source = *sourceOpt;
//     std::cout << "      读取了 " << source.length() << " 字节\n";
    
//     // 2. 汇编
//     std::cout << "[2/3] 汇编中...\n";
    
//     Assembler assembler;
//     AssembleResult result = assembler.assembleWithResult(source);
    
//     if (!result.success) {
//         std::cerr << "\n汇编失败:\n";
//         for (const auto& error : result.errors) {
//             std::cerr << "  " << error << "\n";
//         }
//         return 1;
//     }
    
//     std::cout << "      生成了 " << result.imem.size() << " 条指令 (imem)\n";
//     std::cout << "      生成了 " << result.dmem.size() << " 个数据字 (dmem)\n";
    
//     // 打印指令内存预览
//     std::cout << "\n      指令内存预览 (前 10 条):\n";
//     size_t previewCount = std::min(result.imem.size(), size_t(10));
//     for (size_t i = 0; i < previewCount; i++) {
//         std::cout << "        [" << std::setw(3) << i << "] 0x" 
//                   << std::hex << std::setfill('0') << std::setw(8) 
//                   << result.imem[i] << std::dec << "\n";
//     }
//     if (result.imem.size() > 10) {
//         std::cout << "        ... (共 " << result.imem.size() << " 条)\n";
//     }
    
//     // 打印数据内存预览（如果有）
//     if (!result.dmem.empty()) {
//         std::cout << "\n      数据内存预览 (前 10 个):\n";
//         size_t dataPreviewCount = std::min(result.dmem.size(), size_t(10));
//         for (size_t i = 0; i < dataPreviewCount; i++) {
//             std::cout << "        [" << std::setw(3) << i << "] 0x" 
//                       << std::hex << std::setfill('0') << std::setw(8) 
//                       << result.dmem[i] << std::dec;
//             // 如果是可打印 ASCII 字符，显示
//             if (result.dmem[i] >= 32 && result.dmem[i] < 127) {
//                 std::cout << "  ('" << static_cast<char>(result.dmem[i]) << "')";
//             }
//             std::cout << "\n";
//         }
//         if (result.dmem.size() > 10) {
//             std::cout << "        ... (共 " << result.dmem.size() << " 个)\n";
//         }
//     }
    
//     // 3. 输出文件
//     std::cout << "\n[3/3] 生成输出文件...\n";
    
//     try {
//         if (format == "hex" || format == "both") {
//             // imem -> .hex
//             std::string imemHexPath = outputBase + ".hex";
//             assembler.writeHEX(result.imem, imemHexPath);
//             std::cout << "      已生成 imem: " << imemHexPath << "\n";
            
//             // dmem -> _dmem.hex（如果有数据段）
//             if (!result.dmem.empty()) {
//                 std::string dmemHexPath = outputBase + "_dmem.hex";
//                 assembler.writeHEX(result.dmem, dmemHexPath);
//                 std::cout << "      已生成 dmem: " << dmemHexPath << "\n";
//             }
//         }
        
//         if (format == "mif" || format == "both") {
//             // imem -> .mif
//             std::string imemMifPath = outputBase + ".mif";
//             assembler.writeMIF(result.imem, imemMifPath);
//             std::cout << "      已生成 imem: " << imemMifPath << "\n";
            
//             // dmem -> _dmem.mif（如果有数据段）
//             if (!result.dmem.empty()) {
//                 std::string dmemMifPath = outputBase + "_dmem.mif";
//                 assembler.writeMIF(result.dmem, dmemMifPath);
//                 std::cout << "      已生成 dmem: " << dmemMifPath << "\n";
//             }
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "错误: " << e.what() << "\n";
//         return 1;
//     }
    
//     std::cout << "\n============================================\n";
//     std::cout << "   汇编成功！\n";
//     std::cout << "============================================\n";
    
//     return 0;
// }









#include <iostream>
#include <string>
#include <vector>

#include "compiler/compiler.hpp"
#include "utils/file_io.hpp"

// 打印使用说明
void printUsage(const char *prog) {
    std::cerr << "Usage: " << prog << " <input.c> -o <output.s>\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << prog << " tests/programs/hello_world.c -o hello.s\n";
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputPath;
    std::string outputPath;

    // 解析命令行参数
    inputPath = argv[1];
    std::string flag = argv[2];
    if (flag != "-o") {
        printUsage(argv[0]);
        return 1;
    }
    outputPath = argv[3];

    // ============================
    // 1. 读取 C 源文件
    // ============================
    auto inputOpt = utils::FileIO::readFile(inputPath);
    if (!inputOpt.has_value()) {
        std::cerr << "[Error] Failed to read input file: " << inputPath << "\n";
        return 1;
    }
    std::string sourceCode = *inputOpt;

    // ============================
    // 2. 调用 Compiler（C → Assembly）
    // ============================
    Compiler compiler;

    std::string asmText;
    try {
        asmText = compiler.compileToAssembly(sourceCode);
    } catch (const std::exception &e) {
        std::cerr << "[Compiler Error] " << e.what() << "\n";
        return 1;
    }

    // ============================
    // 3. 输出 Assembly 文件
    // ============================
    auto result = utils::FileIO::writeFile(outputPath, asmText);
    if (!result) {
        std::cerr << "[Error] Failed to write output file: " 
                  << outputPath << "\n";
        std::cerr << "Reason: " << result.error << "\n";
        return 1;
    }

    std::cout << "Compilation successful!\n";
    std::cout << "Generated assembly: " << outputPath << "\n";

    return 0;
}
