
#include "../include/parser.h"
#include <iostream>
#include <cstdlib>
#include <unordered_map>

std::unordered_map<std::string, double> environment;

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
    return nullptr;
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Expr> Parser::expression() {
    return term();
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
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(tokens[current - 1]);
    }
    return nullptr;
}

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
    if (check(type)) return advance();
    std::cerr << "Parser error: " << message << "\n";
    std::exit(1);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

void LiteralExpr::print() const {
    std::cout << "Literal(" << value.lexeme << ")";
}

double LiteralExpr::evaluate() const {
    try {
        return std::stod(value.lexeme);
    } catch (...) {
        return 0.0;
    }
}

void VariableExpr::print() const {
    std::cout << "Variable(" << name.lexeme << ")";
}

double VariableExpr::evaluate() const {
    if (environment.count(name.lexeme)) {
        return environment[name.lexeme];
    }
    std::cerr << "Runtime error: Undefined variable '" << name.lexeme << "'\n";
    std::exit(1);
}

void BinaryExpr::print() const {
    std::cout << "BinaryExpr(";
    left->print();
    std::cout << " " << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

double BinaryExpr::evaluate() const {
    double leftVal = left->evaluate();
    double rightVal = right->evaluate();
    switch (op.type) {
        case TokenType::PLUS: return leftVal + rightVal;
        case TokenType::MINUS: return leftVal - rightVal;
        case TokenType::STAR: return leftVal * rightVal;
        case TokenType::SLASH: return rightVal != 0 ? leftVal / rightVal : 0;
        default: return 0;
    }
}

void VarDecl::execute() const {
    double value = initializer ? initializer->evaluate() : 0.0;
    environment[name.lexeme] = value;
    std::cout << "Declare " << name.lexeme << " = " << value << std::endl;
}

void PrintStmt::execute() const {
    std::cout << "Print: ";
    expr->print();
    std::cout << " => " << expr->evaluate() << std::endl;
}
