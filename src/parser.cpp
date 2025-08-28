#include "../include/parser.h"
#include "../include/environment.h"
#include <iostream>
#include <cstdlib>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        auto decl = declaration();
        if (decl) statements.push_back(std::move(decl));
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({TokenType::VAR, TokenType::LET})) return varDeclaration();
    return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    std::unique_ptr<Expr> initializer;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VarDecl>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::ASSEMBLE})) return assembleStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::LEFT_BRACE})) return parseBlock();
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    std::unique_ptr<Stmt> thenBranch;
    if (match({TokenType::LEFT_BRACE})) {
        thenBranch = parseBlock();
    } else {
        thenBranch = statement();
    }

    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        if (match({TokenType::LEFT_BRACE})) {
            elseBranch = parseBlock();
        } else {
            elseBranch = statement();
        }
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseBlock() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::assembleStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after assemble expression.");
    return std::make_unique<AssembleStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

//Expression Parsing

std::unique_ptr<Expr> Parser::expression() {
    return equality();
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = tokens[current - 1];
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = tokens[current - 1];
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = tokens[current - 1];
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = primary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = tokens[current - 1];
        auto right = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::NUMBER})) {
        return std::make_unique<LiteralExpr>(tokens[current - 1]);
    }
    if (match({TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(tokens[current - 1]);
    }
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(tokens[current - 1]);
    }
    return nullptr;
}


//Parser Utilities

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::consume(TokenType type, const std::string& message) {
    std::cout << "Expecting token: " << static_cast<int>(type)
              << ", current: " << static_cast<int>(peek().type) << "\n";
    if (check(type)) return advance();
    throw std::runtime_error("Parser error: " + message);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

// Expression AST

void LiteralExpr::print() const {
    std::cout << "Literal(" << value.lexeme << ")";
}

double LiteralExpr::evaluate(Environment&) const {
    try {
        return std::stod(value.lexeme);
    } catch (...) {
        return 0.0;
    }
}

void VariableExpr::print() const {
    std::cout << "Variable(" << name.lexeme << ")";
}

double VariableExpr::evaluate(Environment& env) const {
    try {
        return env.get(name.lexeme);
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        std::exit(1);
    }
}

void BinaryExpr::print() const {
    std::cout << "BinaryExpr(";
    left->print();
    std::cout << " " << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

double BinaryExpr::evaluate(Environment& env) const {
    double leftVal = left->evaluate(env);
    double rightVal = right->evaluate(env);

    switch (op.type) {
        case TokenType::PLUS:  return leftVal + rightVal;
        case TokenType::MINUS: return leftVal - rightVal;
        case TokenType::STAR:  return leftVal * rightVal;
        case TokenType::SLASH: return rightVal != 0 ? leftVal / rightVal : 0;
        case TokenType::GREATER:       return leftVal > rightVal ? 1.0 : 0.0;
        case TokenType::GREATER_EQUAL: return leftVal >= rightVal ? 1.0 : 0.0;
        case TokenType::LESS:          return leftVal < rightVal ? 1.0 : 0.0;
        case TokenType::LESS_EQUAL:    return leftVal <= rightVal ? 1.0 : 0.0;
        case TokenType::EQUAL_EQUAL:   return leftVal == rightVal ? 1.0 : 0.0;
        case TokenType::BANG_EQUAL:    return leftVal != rightVal ? 1.0 : 0.0;
        default: return 0;
    }
}

//Statement Execution

void VarDecl::execute(Environment& env) const {
    double value = initializer ? initializer->evaluate(env) : 0.0;
    env.define(name.lexeme, value);
    std::cout << "Declare " << name.lexeme << " = " << value << std::endl;
}

void PrintStmt::execute(Environment& env) const {
    std::cout << "Print: ";
    expr->print();
    std::cout << " => " << expr->evaluate(env) << std::endl;
}

void AssembleStmt::execute(Environment& env) const {
    std::cout << "[Assembler] ";
    expr->print();
    std::cout << " => " << expr->evaluate(env) << std::endl;
}

void IfStmt::execute(Environment& env) const {
    if (condition->evaluate(env)) {
        thenBranch->execute(env);
    } else if (elseBranch) {
        elseBranch->execute(env);
    }
}
