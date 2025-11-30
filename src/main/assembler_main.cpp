/**
 * @file assembler_main.cpp
 * @brief 汇编器独立入口
 * 
 * 提供汇编器的命令行接口，将汇编源文件转换为机器码文件。
 * 
 * 使用方法:
 *   ./assembler <input.s> -o <output_base> [-f format]
 * 
 * 参数:
 *   input.s       汇编源文件
 *   -o output     输出文件名（不含扩展名）
 *   -f format     输出格式: hex, mif, both (默认 both)
 * 
 * 示例:
 *   ./assembler test.s -o test           # 输出 test.hex 和 test.mif
 *   ./assembler test.s -o test -f hex    # 只输出 test.hex
 */

#include <iostream>
#include <string>

#include "assembler/assembler.hpp"
#include "utils/file_io.hpp"

// 打印使用说明
void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " <input.s> -o <output_base> [-f format]\n";
    std::cerr << "Options:\n";
    std::cerr << "  -o output     输出文件名（不含扩展名）\n";
    std::cerr << "  -f format     输出格式: hex, mif, both (默认 both)\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << prog << " test.s -o test\n";
    std::cerr << "  " << prog << " test.s -o test -f hex\n";
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputPath;
    std::string outputBase;
    std::string format = "both";

    // 解析命令行参数
    inputPath = argv[1];
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            outputBase = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            format = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }

    if (outputBase.empty()) {
        std::cerr << "[Error] Missing output path. Use -o <output_base>\n";
        printUsage(argv[0]);
        return 1;
    }

    // ============================
    // 1. 读取汇编源文件
    // ============================
    auto inputOpt = utils::FileIO::readFile(inputPath);
    if (!inputOpt.has_value()) {
        std::cerr << "[Error] Failed to read input file: " << inputPath << "\n";
        return 1;
    }
    std::string asmText = *inputOpt;

    // ============================
    // 2. 调用 Assembler（Assembly → Machine Code）
    // ============================
    assembler::Assembler assembler;
    assembler::AssembleResult result;

    try {
        result = assembler.assemble(asmText);
    } catch (const std::exception& e) {
        std::cerr << "[Assembler Error] " << e.what() << "\n";
        return 1;
    }

    if (!result.success) {
        std::cerr << "[Assembler Error] Assembly failed:\n";
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << "\n";
        }
        return 1;
    }

    // 打印警告
    for (const auto& warning : result.warnings) {
        std::cerr << "[Warning] " << warning << "\n";
    }

    std::cout << "Assembly successful!\n";
    std::cout << "  Generated " << result.imem.size() << " instructions (imem)\n";
    std::cout << "  Generated " << result.dmem.size() << " data words (dmem)\n";

    // ============================
    // 3. 输出机器码文件
    // ============================
    if (format == "hex" || format == "both") {
        // 输出 imem.hex
        std::string imemHexPath = outputBase + ".hex";
        auto hexResult = utils::FileIO::writeHex(imemHexPath, result.imem, 0, true);
        if (!hexResult.success) {
            std::cerr << "[Error] Failed to write HEX file: " << hexResult.error << "\n";
            return 1;
        }
        std::cout << "  Written: " << imemHexPath << "\n";

        // 输出 dmem.hex（如果有数据段）
        if (!result.dmem.empty()) {
            std::string dmemHexPath = outputBase + "_dmem.hex";
            auto dmemHexResult = utils::FileIO::writeHex(dmemHexPath, result.dmem, 0, true);
            if (!dmemHexResult.success) {
                std::cerr << "[Error] Failed to write dmem HEX file: " << dmemHexResult.error << "\n";
                return 1;
            }
            std::cout << "  Written: " << dmemHexPath << "\n";
        }
    }

    if (format == "mif" || format == "both") {
        // 输出 imem.mif
        std::string imemMifPath = outputBase + ".mif";
        auto mifResult = utils::FileIO::writeMif(imemMifPath, result.imem);
        if (!mifResult.success) {
            std::cerr << "[Error] Failed to write MIF file: " << mifResult.error << "\n";
            return 1;
        }
        std::cout << "  Written: " << imemMifPath << "\n";

        // 输出 dmem.mif（如果有数据段）
        if (!result.dmem.empty()) {
            std::string dmemMifPath = outputBase + "_dmem.mif";
            auto dmemMifResult = utils::FileIO::writeMif(dmemMifPath, result.dmem);
            if (!dmemMifResult.success) {
                std::cerr << "[Error] Failed to write dmem MIF file: " << dmemMifResult.error << "\n";
                return 1;
            }
            std::cout << "  Written: " << dmemMifPath << "\n";
        }
    }

    return 0;
}
