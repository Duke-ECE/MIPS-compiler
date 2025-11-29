#pragma once
#include <string>
#include <vector>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "semantic_analyzer.hpp"
#include "ir.hpp"
#include "ir_builder.hpp"
#include "optimizer.hpp"
#include "codegen.hpp"

class Compiler {
public:
    // C → Assembly text
    std::string compileToAssembly(const std::string &sourceCode);

private:
    // Internal pipeline helpers
    std::vector<Token> runLexer(const std::string &code);
    std::unique_ptr<ASTProgram> runParser(const std::vector<Token>& tokens);
    void runSemanticAnalysis(ASTProgram *program);
    IRProgram runIR(const ASTProgram *program);
    IRProgram runOptimizer(const IRProgram &program);
    std::vector<std::string> runCodeGen(const IRProgram &program);
};
