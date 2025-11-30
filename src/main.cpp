/**
 * @file main.cpp
 * @brief 默认主入口（兼容性）
 * 
 * 这是一个简单的主入口，用于向后兼容。
 * 实际使用时，请使用以下专用工具：
 *   - toolchain: 统一工具链
 *   - compiler: 编译器
 *   - assembler: 汇编器
 */

#include <iostream>

int main(int argc, char** argv) {
    std::cout << "MIPS-compiler 工具链\n";
    std::cout << "===================\n\n";
    std::cout << "可用工具:\n";
    std::cout << "  toolchain  - 统一工具链入口\n";
    std::cout << "  compiler   - C 编译器\n";
    std::cout << "  assembler  - 汇编器\n\n";
    std::cout << "请使用具体工具，例如:\n";
    std::cout << "  ./toolchain -c test.c -o test.hex\n";
    std::cout << "  ./compiler test.c -o test.s\n";
    std::cout << "  ./assembler test.s -o test.hex\n";
    
    return 0;
}
