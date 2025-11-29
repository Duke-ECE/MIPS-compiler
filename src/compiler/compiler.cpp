#include "compiler/compiler.hpp"
#include <sstream>
#include <stdexcept>

/*****************************************
 * 1. Lexer 阶段
 *****************************************/
std::vector<Token> Compiler::runLexer(const std::string &code) {
    try {
        Lexer lex(code);
        return lex.tokenize();   // 使用你提供的 tokenize()
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Lexer error: ") + e.what());
    }
}

/*****************************************
 * 2. Parser 阶段
 *****************************************/
std::unique_ptr<ASTProgram>
Compiler::runParser(const std::vector<Token> &tokens) {
    try {
        Parser parser(tokens);
        return parser.parseProgram();   // 返回 unique_ptr<ASTProgram>
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Parser error: ") + e.what());
    }
}

/*****************************************
 * 3. Semantic Analyzer 阶段
 *****************************************/
void Compiler::runSemanticAnalysis(ASTProgram *program) {
    try {
        SemanticAnalyzer analyzer;
        analyzer.analyze(program);   // 类型检查、作用域检查
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Semantic error: ") + e.what());
    }
}

/*****************************************
 * 4. IRBuilder 阶段 (AST → IRProgram)
 *****************************************/
IRProgram Compiler::runIR(const ASTProgram *program) {
    try {
        IRBuilder builder;
        return builder.generateIR(program);  // 参数是 ASTNode*
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("IRBuilder error: ") + e.what());
    }
}

/*****************************************
 * 5. Optimizer 阶段 (IR → 优化的IR)
 *****************************************/
IRProgram Compiler::runOptimizer(const IRProgram &program) {
    try {
        Optimizer opt;
        return opt.optimize(program);
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Optimizer error: ") + e.what());
    }
}

/*****************************************
 * 6. CodeGen 阶段 (IR → Assembly)
 *****************************************/
std::vector<std::string>
Compiler::runCodeGen(const IRProgram &program) {
    try {
        CodeGen gen;
        return gen.generateAssembly(program);
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("CodeGen error: ") + e.what());
    }
}

/*****************************************
 * 6. 顶层接口：C → Assembly
 *****************************************/
std::string Compiler::compileToAssembly(const std::string &sourceCode) {
    try {
        // ---------- 前端 ----------
        auto tokens = runLexer(sourceCode);
        auto ast = runParser(tokens);
        runSemanticAnalysis(ast.get());

        // ---------- 中端 ----------
        IRProgram ir = runIR(ast.get());
        
        // ---------- 优化 ----------
        // 临时禁用优化器
        IRProgram optimizedIR = runOptimizer(ir);

        // ---------- 后端 ----------
        // auto asmLines = runCodeGen(ir);  // 使用未优化的IR
        auto asmLines = runCodeGen(optimizedIR);// 使用优化后的IR

        // ---------- 合并成字符串 ----------
        std::ostringstream out;
        for (const auto &line : asmLines) {
            out << line << "\n";
        }
        return out.str();

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Compiler failed: ") + e.what());
    }
}
