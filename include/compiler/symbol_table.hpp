/*
symbol_table.hpp: MIPS 编译器的符号表定义
作用： 定义支持多级作用域的符号表类，用于存储变量和函数的声明信息。
*/

#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>

// 符号表：支持多级作用域
class SymbolTable {
public:
    SymbolTable();

    // 进入一个新的作用域
    void enterScope();

    // 离开当前作用域
    void exitScope();

    // 声明一个变量/函数（返回 false = 重复声明）
    bool declare(const std::string &name, const std::string &type);

    // 从当前作用域开始查找符号（“向上查找”）
    // 找不到时返回 ""（空字符串）
    std::string lookup(const std::string &name) const;

private:
    // 栈顶 = 当前作用域
    std::vector<std::unordered_map<std::string, std::string>> scopes;
};

#endif // SYMBOL_TABLE_HPP
