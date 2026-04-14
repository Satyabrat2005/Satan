#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/environment.h"
#include <iostream>
#include <cstdlib>
#include <stdexcept>

static constexpr int MAX_LOOP_ITERATIONS = 1000000;

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
    if (match({TokenType::FUNC})) return funDeclaration();
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

std::unique_ptr<Stmt> Parser::funDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");
    auto body = std::shared_ptr<BlockStmt>(static_cast<BlockStmt*>(parseBlock().release()));

    return std::make_unique<FunDecl>(name, std::move(parameters), std::move(body));
}

// ----------------- Statements -----------------
std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::ASSEMBLE})) return assembleStatement();
    if (match({TokenType::SUMMON})) return summonStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::BREAK})) return breakStatement();
    if (match({TokenType::CONTINUE})) return continueStatement();
    if (match({TokenType::RETURN})) return returnStatement();
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

    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> initializer;
    if (match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if (match({TokenType::VAR})) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    auto body = statement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::breakStatement() {
    consume(TokenType::SEMICOLON, "Expect ';' after 'break'.");
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::continueStatement() {
    consume(TokenType::SEMICOLON, "Expect ';' after 'continue'.");
    return std::make_unique<ContinueStmt>();
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

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = tokens[current - 1];
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

// ----------------- Expressions -----------------
std::unique_ptr<Expr> Parser::expression() {
    return logical();
}

std::unique_ptr<Expr> Parser::logical() {
    auto expr = equality();
    while (match({TokenType::AND, TokenType::OR})) {
        Token op = tokens[current - 1];
        auto right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
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
    auto expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = tokens[current - 1];
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = tokens[current - 1];
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    auto expr = primary();
    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } else {
            break;
        }
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
    throw std::runtime_error("Parser error at line " + std::to_string(peek().line) + ": unexpected token '" + peek().lexeme + "'");
}

// ----------------- Helpers -----------------
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
    throw std::runtime_error("Parser error: " + message);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

// ----------------- Expression Implementations -----------------

void LiteralExpr::print() const {
    std::cout << value.lexeme;
}

double LiteralExpr::evaluate(Environment& env) const {
    if (value.type == TokenType::NUMBER) {
        return std::stod(value.lexeme);
    }
    if (value.type == TokenType::TRUE) return 1.0;
    if (value.type == TokenType::FALSE) return 0.0;
    if (value.type == TokenType::STRING) {
        // Strings can't be represented as double; return 0 for now
        // but printing is handled by SummonStmt/PrintStmt
        return 0.0;
    }
    return 0.0;
}

void VariableExpr::print() const {
    std::cout << name.lexeme;
}

double VariableExpr::evaluate(Environment& env) const {
    return env.getNumber(name.lexeme);
}

void BinaryExpr::print() const {
    std::cout << "(";
    left->print();
    std::cout << " " << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

double BinaryExpr::evaluate(Environment& env) const {
    double l = left->evaluate(env);
    double r = right->evaluate(env);
    switch (op.type) {
        case TokenType::PLUS:          return l + r;
        case TokenType::MINUS:         return l - r;
        case TokenType::STAR:          return l * r;
        case TokenType::SLASH:
            if (r == 0) throw std::runtime_error("Division by zero.");
            return l / r;
        case TokenType::GREATER:       return l > r ? 1.0 : 0.0;
        case TokenType::GREATER_EQUAL: return l >= r ? 1.0 : 0.0;
        case TokenType::LESS:          return l < r ? 1.0 : 0.0;
        case TokenType::LESS_EQUAL:    return l <= r ? 1.0 : 0.0;
        case TokenType::EQUAL_EQUAL:   return l == r ? 1.0 : 0.0;
        case TokenType::BANG_EQUAL:    return l != r ? 1.0 : 0.0;
        default:
            throw std::runtime_error("Unknown binary operator: " + op.lexeme);
    }
}

void CallExpr::print() const {
    callee->print();
    std::cout << "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) std::cout << ", ";
        arguments[i]->print();
    }
    std::cout << ")";
}

double CallExpr::evaluate(Environment& env) const {
    // Get function name from callee
    auto* varExpr = dynamic_cast<const VariableExpr*>(callee.get());
    if (!varExpr) throw std::runtime_error("Can only call named functions.");

    FunctionObject func = env.getFunction(varExpr->name.lexeme);

    if (arguments.size() != func.params.size()) {
        throw std::runtime_error("Expected " + std::to_string(func.params.size()) +
                                 " arguments but got " + std::to_string(arguments.size()) + ".");
    }

    Environment callEnv(&env);
    for (size_t i = 0; i < func.params.size(); i++) {
        double argVal = arguments[i]->evaluate(env);
        callEnv.define(func.params[i], argVal);
    }

    try {
        func.body->execute(callEnv);
    } catch (const ReturnException& ret) {
        return ret.value;
    }
    return 0.0;
}

// ----------------- Statement Implementations -----------------

void VarDecl::execute(Environment& env) const {
    double val = 0.0;
    if (initializer) {
        val = initializer->evaluate(env);
    }
    env.define(name.lexeme, val);
}

void AssembleStmt::execute(Environment& env) const {
    double val = expr->evaluate(env);
    std::cout << val << std::endl;
}

void PrintStmt::execute(Environment& env) const {
    double val = expr->evaluate(env);
    std::cout << val << std::endl;
}

void IfStmt::execute(Environment& env) const {
    double condVal = condition->evaluate(env);
    if (condVal != 0.0) {
        thenBranch->execute(env);
    } else if (elseBranch) {
        elseBranch->execute(env);
    }
}

void BlockStmt::execute(Environment& env) const {
    Environment blockEnv(&env);
    for (const auto& stmt : statements) {
        stmt->execute(blockEnv);
    }
}

void ExprStmt::execute(Environment& env) const {
    expr->evaluate(env);
}

void SummonStmt::execute(Environment& env) const {
    // SummonStmt prints a message — check if it's a string literal
    auto* lit = dynamic_cast<const LiteralExpr*>(message.get());
    if (lit && lit->value.type == TokenType::STRING) {
        std::cout << lit->value.lexeme << std::endl;
    } else {
        double val = message->evaluate(env);
        std::cout << val << std::endl;
    }
}

void FunDecl::execute(Environment& env) const {
    FunctionObject func;
    for (const auto& param : params) {
        func.params.push_back(param.lexeme);
    }
    // Share ownership of the body so the FunctionObject keeps it alive
    // even after the FunDecl statement is destroyed (e.g., in REPL mode)
    func.body = body;
    env.defineFunction(name.lexeme, func);
}

void ReturnStmt::execute(Environment& env) const {
    double val = 0.0;
    if (value) {
        val = value->evaluate(env);
    }
    throw ReturnException(val);
}

// ----------------- Parser summonStatement -----------------

std::unique_ptr<Stmt> Parser::summonStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after summon expression.");
    return std::make_unique<SummonStmt>(std::move(expr));
}

void UnaryExpr::print() const {
    std::cout << "Unary(" << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

double UnaryExpr::evaluate(Environment& env) const {
    double val = right->evaluate(env);
    if (op.type == TokenType::MINUS) return -val;
    if (op.type == TokenType::BANG) return val == 0 ? 1.0 : 0.0;
    return val;
}

void LogicalExpr::print() const {
    std::cout << "Logical(";
    left->print();
    std::cout << " " << op.lexeme << " ";
    right->print();
    std::cout << ")";
}

double LogicalExpr::evaluate(Environment& env) const {
    double leftVal = left->evaluate(env);
    if (op.type == TokenType::OR) {
        if (leftVal) return 1.0;
        return right->evaluate(env);
    }
    if (op.type == TokenType::AND) {
        if (!leftVal) return 0.0;
        return right->evaluate(env);
    }
    return 0.0;
}

void WhileStmt::execute(Environment& env) const {
    int iterations = 0;
    while (condition->evaluate(env)) {
        if (++iterations > MAX_LOOP_ITERATIONS) {
            throw std::runtime_error("While loop exceeded maximum iteration limit");
        }
        try {
            body->execute(env);
        } catch (const std::runtime_error& e) {
            if (std::string(e.what()) == "Break") break;
            if (std::string(e.what()) == "Continue") continue;
            throw;
        }
    }
}

void ForStmt::execute(Environment& env) const {
    if (initializer) initializer->execute(env);
    int iterations = 0;
    while (!condition || condition->evaluate(env)) {
        if (++iterations > MAX_LOOP_ITERATIONS) {
            throw std::runtime_error("For loop exceeded maximum iteration limit");
        }
        try {
            body->execute(env);
        } catch (const std::runtime_error& e) {
            if (std::string(e.what()) == "Break") break;
            if (std::string(e.what()) == "Continue") {
                if (increment) increment->evaluate(env);
                continue;
            }
            throw;
        }
        if (increment) increment->evaluate(env);
    }
}

void BreakStmt::execute(Environment&) const {
    throw std::runtime_error("Break");
}

void ContinueStmt::execute(Environment&) const {
    throw std::runtime_error("Continue");
}
