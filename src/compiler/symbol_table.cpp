#include "compiler/symbol_table.hpp"
#include <stdexcept>

SymbolTable::SymbolTable() {
    // 初始无作用域，由外部显式 enterScope()
}

void SymbolTable::enterScope() {
    scopes.emplace_back();  // push 一个新的空 map
}

void SymbolTable::exitScope() {
    if (scopes.empty()) {
        throw std::runtime_error("SymbolTable: exitScope() with empty scope stack");
    }
    scopes.pop_back();
}

bool SymbolTable::declare(const std::string &name, const std::string &type) {
    if (scopes.empty()) {
        throw std::runtime_error("SymbolTable: declare() called with no active scope");
    }

    auto &current = scopes.back();
    if (current.count(name)) {
        return false;  // duplicate declaration
    }

    current[name] = type;
    return true;
}

std::string SymbolTable::lookup(const std::string &name) const {
    // 从当前作用域往上查
    for (int i = scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return it->second;    // return type string
        }
    }
    return "";  // not found
}
