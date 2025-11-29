#include "compiler/parser.hpp"

// ========== 构造函数 ==========

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), current(0) {}

// ========== 基础工具 ==========

const Token &Parser::peek(int offset) const {
    size_t index = current + offset;
    if (index >= tokens.size()) {
        return tokens.back(); // 假设最后一个是 EOF
    }
    return tokens[index];
}

const Token &Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token &Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token &Parser::consume(TokenType type, const std::string &errMsg) {
    if (check(type)) return advance();
    throw ParseError(errMsg + " (got token: " + peek().text + ")");
}

// ========== 顶层：Program / Function ==========

std::unique_ptr<ASTProgram> Parser::parseProgram() {
    auto program = std::make_unique<ASTProgram>();

    while (!isAtEnd()) {
        program->functions.push_back(parseFunction());
    }

    return program;
}

// function_definition → "int" IDENT "(" parameters? ")" block
std::unique_ptr<ASTFunction> Parser::parseFunction() {
    // 目前只支持 int 返回类型
    consume(TokenType::INT, "expected 'int' at function declaration");

    const Token &nameTok =
        consume(TokenType::IDENTIFIER, "expected function name");

    auto func = std::make_unique<ASTFunction>();
    func->returnType = "int";
    func->name = nameTok.text;

    consume(TokenType::LPAREN, "expected '(' after function name");

    // 参数列表
    if (!check(TokenType::RPAREN)) {
        parseParameterList(*func);
    }

    consume(TokenType::RPAREN, "expected ')' after parameter list");

    // 函数体
    func->body = parseBlock();

    return func;
}

// parameters → "int" IDENT ("," "int" IDENT)* | ε
void Parser::parseParameterList(ASTFunction &func) {
    // 第一个参数
    consume(TokenType::INT, "expected 'int' in parameter list");
    const Token &firstName =
        consume(TokenType::IDENTIFIER, "expected parameter name");

    func.paramTypes.push_back("int");
    func.paramNames.push_back(firstName.text);

    // 后续参数
    while (match(TokenType::COMMA)) {
        consume(TokenType::INT, "expected 'int' in parameter list");
        const Token &nameTok =
            consume(TokenType::IDENTIFIER, "expected parameter name");
        func.paramTypes.push_back("int");
        func.paramNames.push_back(nameTok.text);
    }
}

// ========== Block / Statement ==========

// block → "{" statement* "}"
std::unique_ptr<ASTBlock> Parser::parseBlock() {
    consume(TokenType::LBRACE, "expected '{' to start block");

    auto block = std::make_unique<ASTBlock>();

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "expected '}' after block");

    return block;
}

// statement → block
//           | declaration
//           | if_statement
//           | while_statement
//           | return_statement
//           | expression_statement
std::unique_ptr<ASTStatement> Parser::parseStatement() {
    if (check(TokenType::LBRACE)) {
        return parseBlock();
    }

    if (check(TokenType::INT)) {
        return parseDeclarationStatement();
    }

    if (check(TokenType::IF)) {
        return parseIfStatement();
    }

    if (check(TokenType::WHILE)) {
        return parseWhileStatement();
    }

    if (check(TokenType::RETURN)) {
        return parseReturnStatement();
    }

    return parseExpressionStatement();
}

// declaration → "int" IDENT ("=" expression)? ";"
std::unique_ptr<ASTStatement> Parser::parseDeclarationStatement() {
    consume(TokenType::INT, "expected 'int' in declaration");
    const Token &nameTok =
        consume(TokenType::IDENTIFIER, "expected variable name");

    std::unique_ptr<ASTExpression> initExpr = nullptr;
    if (match(TokenType::ASSIGN)) {
        initExpr = parseExpression();
    }

    consume(TokenType::SEMICOLON, "expected ';' after declaration");

    auto decl = std::make_unique<ASTVarDecl>();
    decl->typeName = "int";
    decl->name = nameTok.text;
    decl->initExpr = std::move(initExpr);
    return decl;
}

// if_statement → "if" "(" expression ")" statement ( "else" statement )?
std::unique_ptr<ASTStatement> Parser::parseIfStatement() {
    consume(TokenType::IF, "expected 'if'");
    consume(TokenType::LPAREN, "expected '(' after 'if'");

    auto cond = parseExpression();

    consume(TokenType::RPAREN, "expected ')' after if condition");

    auto thenBranch = parseStatement();

    std::unique_ptr<ASTStatement> elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement();
    }

    auto node = std::make_unique<ASTIf>();
    node->condition = std::move(cond);
    node->thenBranch = std::move(thenBranch);
    node->elseBranch = std::move(elseBranch);

    return node;
}

// while_statement → "while" "(" expression ")" statement
std::unique_ptr<ASTStatement> Parser::parseWhileStatement() {
    consume(TokenType::WHILE, "expected 'while'");
    consume(TokenType::LPAREN, "expected '(' after 'while'");

    auto cond = parseExpression();

    consume(TokenType::RPAREN, "expected ')' after while condition");

    auto body = parseStatement();

    auto node = std::make_unique<ASTWhile>();
    node->condition = std::move(cond);
    node->body = std::move(body);
    return node;
}

// return_statement → "return" expression? ";"
std::unique_ptr<ASTStatement> Parser::parseReturnStatement() {
    consume(TokenType::RETURN, "expected 'return'");

    std::unique_ptr<ASTExpression> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }

    consume(TokenType::SEMICOLON, "expected ';' after return statement");

    auto node = std::make_unique<ASTReturn>();
    node->value = std::move(value);
    return node;
}

// expression_statement → expression ";"
std::unique_ptr<ASTStatement> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "expected ';' after expression");

    auto stmt = std::make_unique<ASTExprStmt>();
    stmt->expr = std::move(expr);
    return stmt;
}

// ========== 表达式解析（带优先级） ==========

// expression → assignment
std::unique_ptr<ASTExpression> Parser::parseExpression() {
    return parseAssignment();
}

// assignment → IDENT "=" assignment | equality
std::unique_ptr<ASTExpression> Parser::parseAssignment() {
    auto left = parseEquality();

    if (match(TokenType::ASSIGN)) {
        // 左边必须是 Identifier
        auto *id = dynamic_cast<ASTIdentifierExpr *>(left.get());
        if (!id) {
            throw ParseError("Left-hand side of assignment must be a variable");
        }

        std::string name = id->name;
        auto value = parseAssignment();

        auto assign = std::make_unique<ASTAssignExpr>();
        assign->name = name;
        assign->value = std::move(value);
        return assign;
    }

    return left;
}

// equality → comparison ( ("==" | "!=") comparison )*
std::unique_ptr<ASTExpression> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match({TokenType::EQ, TokenType::NEQ})) {
        TokenType op = previous().type;
        auto right = parseComparison();

        ASTBinaryOpKind kind;
        if (op == TokenType::EQ) {
            kind = ASTBinaryOpKind::Equal;
        } else {
            kind = ASTBinaryOpKind::NotEqual;
        }

        auto bin = std::make_unique<ASTBinaryExpr>();
        bin->op = kind;
        bin->left = std::move(expr);
        bin->right = std::move(right);
        expr = std::move(bin);
    }

    return expr;
}

// comparison → additive ( ("<" | "<=" | ">" | ">=") additive )*
std::unique_ptr<ASTExpression> Parser::parseComparison() {
    auto expr = parseAdditive();

    while (match({TokenType::LT, TokenType::LE,
                  TokenType::GT, TokenType::GE})) {
        TokenType op = previous().type;
        auto right = parseAdditive();

        ASTBinaryOpKind kind;
        switch (op) {
        case TokenType::LT:          kind = ASTBinaryOpKind::Less; break;
        case TokenType::LE:          kind = ASTBinaryOpKind::LessEqual; break;
        case TokenType::GT:          kind = ASTBinaryOpKind::Greater; break;
        case TokenType::GE:          kind = ASTBinaryOpKind::GreaterEqual; break;
        default:
            throw ParseError("unexpected comparison operator");
        }

        auto bin = std::make_unique<ASTBinaryExpr>();
        bin->op = kind;
        bin->left = std::move(expr);
        bin->right = std::move(right);
        expr = std::move(bin);
    }

    return expr;
}

// additive → multiplicative ( ("+" | "-") multiplicative )*
std::unique_ptr<ASTExpression> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        TokenType op = previous().type;
        auto right = parseMultiplicative();

        ASTBinaryOpKind kind =
            (op == TokenType::PLUS) ? ASTBinaryOpKind::Add : ASTBinaryOpKind::Sub;

        auto bin = std::make_unique<ASTBinaryExpr>();
        bin->op = kind;
        bin->left = std::move(expr);
        bin->right = std::move(right);
        expr = std::move(bin);
    }

    return expr;
}

// multiplicative → unary ( ("*" | "/") unary )*
std::unique_ptr<ASTExpression> Parser::parseMultiplicative() {
    auto expr = parseUnary();

    while (match({TokenType::STAR, TokenType::SLASH})) {
        TokenType op = previous().type;
        auto right = parseUnary();

        ASTBinaryOpKind kind =
            (op == TokenType::STAR) ? ASTBinaryOpKind::Mul : ASTBinaryOpKind::Div;

        auto bin = std::make_unique<ASTBinaryExpr>();
        bin->op = kind;
        bin->left = std::move(expr);
        bin->right = std::move(right);
        expr = std::move(bin);
    }

    return expr;
}

// unary → ("!" | "-") unary | primary
std::unique_ptr<ASTExpression> Parser::parseUnary() {
    if (match(TokenType::MINUS)) {
        auto operand = parseUnary();
        auto node = std::make_unique<ASTUnaryExpr>();
        node->op = ASTUnaryOpKind::Neg;
        node->expr = std::move(operand);
        return node;
    }

    return parsePrimary();
}

// primary → NUMBER | IDENT | "(" expression ")" | function_call
std::unique_ptr<ASTExpression> Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) {
        const Token &numTok = previous();
        auto node = std::make_unique<ASTNumberExpr>();
        node->value = std::stoi(numTok.text);
        return node;
    }

    if (match(TokenType::IDENTIFIER)) {
        const Token &idTok = previous();
        std::string name = idTok.text;

        // 函数调用 or 普通变量
        if (match(TokenType::LPAREN)) {
            return finishCall(name);
        }

        auto node = std::make_unique<ASTIdentifierExpr>();
        node->name = name;
        return node;
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "expected ')' after expression");
        return expr;
    }

    throw ParseError("unexpected token in expression: " + peek().text);
}

// function_call → IDENT "(" argument_list? ")"
std::unique_ptr<ASTExpression> Parser::finishCall(const std::string &funcName) {
    std::vector<std::unique_ptr<ASTExpression>> args;

    if (!check(TokenType::RPAREN)) {
        parseArgumentList(args);
    }

    consume(TokenType::RPAREN, "expected ')' after function arguments");

    auto call = std::make_unique<ASTCallExpr>();
    call->callee = funcName;
    call->arguments = std::move(args);
    return call;
}

// argument_list → expression ("," expression)*
void Parser::parseArgumentList(std::vector<std::unique_ptr<ASTExpression>> &args) {
    args.push_back(parseExpression());
    while (match(TokenType::COMMA)) {
        args.push_back(parseExpression());
    }
}