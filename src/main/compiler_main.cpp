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
