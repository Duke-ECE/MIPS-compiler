/**
 * @file toolchain.cpp
 * @brief 统一工具链入口
 * 
 * 提供顶层命令行接口，统一调度编译器和汇编器。
 * 通过内存传递避免不必要的中间文件读写。
 * 
 * 使用方法:
 *   toolchain -s <input.c> -o <output.s>          # C → Assembly
 *   toolchain -c <input.c> -o <output.hex>        # C → Machine Code (hex)
 *   toolchain -a <input.s> -o <output.hex>        # Assembly → Machine Code (hex)
 *   toolchain -c <input.c> -o <output> -f mif     # C → Machine Code (mif)
 * 
 * 参数:
 *   -s          只编译为汇编代码（C → ASM）
 *   -c          完整编译（C → ASM → Machine Code）
 *   -a          只汇编（ASM → Machine Code）
 *   -o output   输出文件路径
 *   -f format   输出格式：hex（默认）或 mif
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "compiler/compiler.hpp"
#include "assembler/assembler.hpp"
#include "utils/file_io.hpp"

// 工作模式
enum class Mode {
    UNKNOWN,
    S_ONLY,     // C → Assembly
    C_FULL,     // C → Assembly → Machine Code
    ASM_ONLY    // Assembly → Machine Code
};

// 输出格式
enum class OutputFormat {
    HEX,
    MIF,
    BOTH
};

// 打印使用说明
void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " [mode] <input> -o <output> [-f format]\n";
    std::cerr << "\n";
    std::cerr << "Modes:\n";
    std::cerr << "  -s          C → Assembly（只编译为汇编代码）\n";
    std::cerr << "  -c          C → Machine Code（完整编译）\n";
    std::cerr << "  -a          Assembly → Machine Code（只汇编）\n";
    std::cerr << "\n";
    std::cerr << "Options:\n";
    std::cerr << "  -o output   输出文件路径\n";
    std::cerr << "  -f format   输出格式: hex（默认）, mif, both\n";
    std::cerr << "\n";
    std::cerr << "Examples:\n";
    std::cerr << "  " << prog << " -s hello.c -o hello.s\n";
    std::cerr << "  " << prog << " -c hello.c -o imem.hex\n";
    std::cerr << "  " << prog << " -a hello.s -o imem.mif -f mif\n";
    std::cerr << "  " << prog << " -c hello.c -o imem -f both\n";
}

// 解析命令行参数
struct Config {
    Mode mode = Mode::UNKNOWN;
    std::string inputPath;
    std::string outputPath;
    OutputFormat format = OutputFormat::HEX;
};

Config parseArgs(int argc, char** argv) {
    Config config;
    
    if (argc < 5) {
        return config;
    }
    
    // 解析模式
    std::string modeStr = argv[1];
    if (modeStr == "-s") {
        config.mode = Mode::S_ONLY;
    } else if (modeStr == "-c") {
        config.mode = Mode::C_FULL;
    } else if (modeStr == "-a") {
        config.mode = Mode::ASM_ONLY;
    } else {
        return config;
    }
    
    // 解析输入文件
    config.inputPath = argv[2];
    
    // 解析其他参数
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-o" && i + 1 < argc) {
            config.outputPath = argv[++i];
        } else if (arg == "-f" && i + 1 < argc) {
            std::string fmt = argv[++i];
            if (fmt == "hex") {
                config.format = OutputFormat::HEX;
            } else if (fmt == "mif") {
                config.format = OutputFormat::MIF;
            } else if (fmt == "both") {
                config.format = OutputFormat::BOTH;
            }
        }
    }
    
    return config;
}

// 主函数
int main(int argc, char** argv) {
    // 解析命令行参数
    Config config = parseArgs(argc, argv);
    
    if (config.mode == Mode::UNKNOWN || config.inputPath.empty() || config.outputPath.empty()) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        // ====================================
        // 模式 1: C → Assembly（-s）
        // ====================================
        if (config.mode == Mode::S_ONLY) {
            std::cout << "[Toolchain] C → Assembly\n";
            
            // 读取 C 源文件
            std::string cSource = utils::FileIO::readText(config.inputPath);
            
            // 编译为汇编代码
            Compiler compiler;
            std::string asmText = compiler.compileToAssembly(cSource);
            
            // 写入汇编文件
            utils::FileIO::writeText(config.outputPath, asmText);
            
            std::cout << "✓ 编译成功！\n";
            std::cout << "  生成汇编文件: " << config.outputPath << "\n";
            return 0;
        }
        
        // ====================================
        // 模式 2: Assembly → Machine Code（-a）
        // ====================================
        if (config.mode == Mode::ASM_ONLY) {
            std::cout << "[Toolchain] Assembly → Machine Code\n";
            
            // 读取汇编文件
            std::string asmText = utils::FileIO::readText(config.inputPath);
            
            // 汇编为机器码
            assembler::Assembler assembler;
            assembler::AssembleResult result = assembler.assemble(asmText);
            
            if (!result.success) {
                std::cerr << "[汇编失败]\n";
                for (const auto& error : result.errors) {
                    std::cerr << "  " << error << "\n";
                }
                return 1;
            }
            
            // 打印警告
            for (const auto& warning : result.warnings) {
                std::cerr << "[Warning] " << warning << "\n";
            }
            
            std::cout << "✓ 汇编成功！\n";
            std::cout << "  生成 " << result.imem.size() << " 条指令\n";
            if (!result.dmem.empty()) {
                std::cout << "  生成 " << result.dmem.size() << " 个数据字\n";
            }
            
            // 根据格式输出文件
            if (config.format == OutputFormat::HEX || config.format == OutputFormat::BOTH) {
                std::string hexPath = config.outputPath;
                if (config.format == OutputFormat::BOTH) {
                    hexPath = utils::FileIO::replaceExtension(config.outputPath, ".hex");
                }
                
                auto hexResult = utils::FileIO::writeHex(hexPath, result.imem, 0, true);
                if (!hexResult.success) {
                    std::cerr << "[Error] 写入 HEX 文件失败: " << hexResult.error << "\n";
                    return 1;
                }
                std::cout << "  输出: " << hexPath << "\n";
                
                // 如果有数据段，也输出 dmem
                if (!result.dmem.empty()) {
                    std::string dmemHexPath = utils::FileIO::replaceExtension(hexPath, "_dmem.hex");
                    auto dmemResult = utils::FileIO::writeHex(dmemHexPath, result.dmem, 0, true);
                    if (!dmemResult.success) {
                        std::cerr << "[Warning] 写入 dmem HEX 文件失败: " << dmemResult.error << "\n";
                    } else {
                        std::cout << "  输出: " << dmemHexPath << "\n";
                    }
                }
            }
            
            if (config.format == OutputFormat::MIF || config.format == OutputFormat::BOTH) {
                std::string mifPath = config.outputPath;
                if (config.format == OutputFormat::BOTH) {
                    mifPath = utils::FileIO::replaceExtension(config.outputPath, ".mif");
                }
                
                auto mifResult = utils::FileIO::writeMif(mifPath, result.imem);
                if (!mifResult.success) {
                    std::cerr << "[Error] 写入 MIF 文件失败: " << mifResult.error << "\n";
                    return 1;
                }
                std::cout << "  输出: " << mifPath << "\n";
                
                // 如果有数据段，也输出 dmem
                if (!result.dmem.empty()) {
                    std::string dmemMifPath = utils::FileIO::replaceExtension(mifPath, "_dmem.mif");
                    auto dmemResult = utils::FileIO::writeMif(dmemMifPath, result.dmem);
                    if (!dmemResult.success) {
                        std::cerr << "[Warning] 写入 dmem MIF 文件失败: " << dmemResult.error << "\n";
                    } else {
                        std::cout << "  输出: " << dmemMifPath << "\n";
                    }
                }
            }
            
            return 0;
        }
        
        // ====================================
        // 模式 3: C → Assembly → Machine Code（-c）
        // 🔥 核心优化：内存传递，避免临时文件
        // ====================================
        if (config.mode == Mode::C_FULL) {
            std::cout << "[Toolchain] C → Assembly → Machine Code（内存流水线）\n";
            
            // 读取 C 源文件
            std::string cSource = utils::FileIO::readText(config.inputPath);
            
            // 1. 编译为汇编代码（内存中）
            Compiler compiler;
            std::string asmText = compiler.compileToAssembly(cSource);
            std::cout << "  ✓ 编译完成\n";
            
            // 2. 汇编为机器码（内存中）
            assembler::Assembler assembler;
            assembler::AssembleResult result = assembler.assemble(asmText);
            
            if (!result.success) {
                std::cerr << "[汇编失败]\n";
                for (const auto& error : result.errors) {
                    std::cerr << "  " << error << "\n";
                }
                return 1;
            }
            
            // 打印警告
            for (const auto& warning : result.warnings) {
                std::cerr << "[Warning] " << warning << "\n";
            }
            
            std::cout << "  ✓ 汇编完成\n";
            std::cout << "  生成 " << result.imem.size() << " 条指令\n";
            if (!result.dmem.empty()) {
                std::cout << "  生成 " << result.dmem.size() << " 个数据字\n";
            }
            
            // 3. 输出机器码文件
            if (config.format == OutputFormat::HEX || config.format == OutputFormat::BOTH) {
                std::string hexPath = config.outputPath;
                if (config.format == OutputFormat::BOTH) {
                    hexPath = utils::FileIO::replaceExtension(config.outputPath, ".hex");
                }
                
                auto hexResult = utils::FileIO::writeHex(hexPath, result.imem, 0, true);
                if (!hexResult.success) {
                    std::cerr << "[Error] 写入 HEX 文件失败: " << hexResult.error << "\n";
                    return 1;
                }
                std::cout << "  输出: " << hexPath << "\n";
                
                // 如果有数据段，也输出 dmem
                if (!result.dmem.empty()) {
                    std::string dmemHexPath = utils::FileIO::replaceExtension(hexPath, "_dmem.hex");
                    auto dmemResult = utils::FileIO::writeHex(dmemHexPath, result.dmem, 0, true);
                    if (!dmemResult.success) {
                        std::cerr << "[Warning] 写入 dmem HEX 文件失败: " << dmemResult.error << "\n";
                    } else {
                        std::cout << "  输出: " << dmemHexPath << "\n";
                    }
                }
            }
            
            if (config.format == OutputFormat::MIF || config.format == OutputFormat::BOTH) {
                std::string mifPath = config.outputPath;
                if (config.format == OutputFormat::BOTH) {
                    mifPath = utils::FileIO::replaceExtension(config.outputPath, ".mif");
                }
                
                auto mifResult = utils::FileIO::writeMif(mifPath, result.imem);
                if (!mifResult.success) {
                    std::cerr << "[Error] 写入 MIF 文件失败: " << mifResult.error << "\n";
                    return 1;
                }
                std::cout << "  输出: " << mifPath << "\n";
                
                // 如果有数据段，也输出 dmem
                if (!result.dmem.empty()) {
                    std::string dmemMifPath = utils::FileIO::replaceExtension(mifPath, "_dmem.mif");
                    auto dmemResult = utils::FileIO::writeMif(dmemMifPath, result.dmem);
                    if (!dmemResult.success) {
                        std::cerr << "[Warning] 写入 dmem MIF 文件失败: " << dmemResult.error << "\n";
                    } else {
                        std::cout << "  输出: " << dmemMifPath << "\n";
                    }
                }
            }
            
            std::cout << "✓ 编译完成！\n";
            return 0;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[Toolchain Error] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
