#include "../include/parser.h"
#include <iostream>
#include <cstdlib>

// Parser constructor
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

// ---- AST Printing ----

void LiteralExpr::print() const {
    std::cout << "Literal(" << value.lexeme << ")\n";
}

void BinaryExpr::print() const {
    std::cout << "BinaryExpr(";
    left->print();
    std::cout << " " << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

// ---- AST Evaluation ----

double LiteralExpr::evaluate() const {
    try {
        return std::stod(value.lexeme);
    } catch (...) {
        return 0.0;
    }
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

// ---- Statement Execution ----

void VarDecl::execute() const {
    std::cout << "Declare " << name.lexeme << " = ";
    if (initializer) {
        std::cout << initializer->evaluate();
    } else {
        std::cout << "nil";
    }
    std::cout << std::endl;
}

void PrintStmt::execute() const {
    std::cout << "Print: ";
    expr->print();
    std::cout << " => " << expr->evaluate() << std::endl;
}
