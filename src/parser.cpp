#include "../include/parser.h"
#include "../include/interpreter.h"
#include "../include/stdlib_ml.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

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
    if (match({TokenType::FUNC, TokenType::FUN})) return funDeclaration();
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
    auto rawBlock = parseBlock().release();
    auto* blockPtr = dynamic_cast<BlockStmt*>(rawBlock);
    if (!blockPtr) {
        delete rawBlock;
        throw std::runtime_error("Parser error: Expected block statement in function body.");
    }
    auto body = std::shared_ptr<BlockStmt>(blockPtr);
    return std::make_unique<FunDecl>(name, std::move(parameters), std::move(body));
}

// =================== Statements ===================
std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::ASSEMBLE})) return assembleStatement();
    if (match({TokenType::SUMMON})) return summonStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forInOrForStatement();
    if (match({TokenType::BREAK})) return breakStatement();
    if (match({TokenType::CONTINUE})) return continueStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::TRY})) return tryCatchStatement();
    if (match({TokenType::IMPORT})) return importStatement();
    if (match({TokenType::ASSERT})) return assertStatement();
    if (match({TokenType::TEST})) return testStatement();
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
    if (match({TokenType::ELSE})) elseBranch = statement();
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
    if (!check(TokenType::SEMICOLON)) condition = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) increment = expression();
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
    if (!check(TokenType::SEMICOLON)) value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::summonStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after summon expression.");
    return std::make_unique<SummonStmt>(std::move(expr));
}

// =================== Expressions ===================
std::unique_ptr<Expr> Parser::expression() {
    DepthGuard guard(depth);
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logical();
    if (match({TokenType::EQUAL})) {
        auto value = assignment();
        auto* varExpr = dynamic_cast<VariableExpr*>(expr.get());
        if (varExpr) {
            return std::make_unique<AssignExpr>(varExpr->name, std::move(value));
        }
        throw std::runtime_error("Invalid assignment target.");
    }
    return expr;
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
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
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
            // Function call
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    // Check for named argument: identifier = expr
                    if (check(TokenType::IDENTIFIER) && current + 1 < (int)tokens.size()
                        && tokens[current + 1].type == TokenType::EQUAL) {
                        // But make sure it's not ==
                        if (current + 2 < (int)tokens.size() && tokens[current + 2].type != TokenType::EQUAL) {
                            Token name = advance();
                            advance(); // consume =
                            auto value = expression();
                            args.push_back(std::make_unique<NamedArgExpr>(name, std::move(value)));
                            continue;
                        }
                    }
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } else if (match({TokenType::DOT})) {
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            if (check(TokenType::LEFT_PAREN)) {
                // Method call: expr.name(args)
                advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RIGHT_PAREN)) {
                    do {
                        if (check(TokenType::IDENTIFIER) && current + 1 < (int)tokens.size()
                            && tokens[current + 1].type == TokenType::EQUAL
                            && (current + 2 >= (int)tokens.size() || tokens[current + 2].type != TokenType::EQUAL)) {
                            Token argName = advance();
                            advance();
                            auto value = expression();
                            args.push_back(std::make_unique<NamedArgExpr>(argName, std::move(value)));
                            continue;
                        }
                        args.push_back(expression());
                    } while (match({TokenType::COMMA}));
                }
                consume(TokenType::RIGHT_PAREN, "Expect ')' after method arguments.");
                expr = std::make_unique<MethodCallExpr>(std::move(expr), name, std::move(args));
            } else {
                // Property access: expr.name
                expr = std::make_unique<MemberAccessExpr>(std::move(expr), name);
            }
        } else if (match({TokenType::LEFT_BRACKET})) {
            // Index access: expr[index]
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::NUMBER})) return std::make_unique<LiteralExpr>(tokens[current - 1]);
    if (match({TokenType::STRING})) return std::make_unique<LiteralExpr>(tokens[current - 1]);
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(tokens[current - 1]);
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(tokens[current - 1]);
    if (match({TokenType::IDENTIFIER})) return std::make_unique<VariableExpr>(tokens[current - 1]);

    // Array literal: [expr, expr, ...]
    if (match({TokenType::LEFT_BRACKET})) {
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if (match({TokenType::LEFT_PAREN})) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    // Dictionary literal: {key: value, ...}
    if (check(TokenType::LEFT_BRACE) && !isAtEnd()) {
        // Peek ahead to distinguish dict from block
        int saved = current;
        advance(); // consume {
        if (check(TokenType::STRING) || check(TokenType::IDENTIFIER) || check(TokenType::NUMBER)) {
            int afterKey = current;
            advance(); // skip key
            if (check(TokenType::COLON)) {
                // It's a dictionary literal!
                current = saved + 1; // back to after {
                std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> entries;
                if (!check(TokenType::RIGHT_BRACE)) {
                    do {
                        auto key = expression();
                        consume(TokenType::COLON, "Expect ':' after dictionary key.");
                        auto value = expression();
                        entries.push_back({std::move(key), std::move(value)});
                    } while (match({TokenType::COMMA}));
                }
                consume(TokenType::RIGHT_BRACE, "Expect '}' after dictionary.");
                return std::make_unique<DictExpr>(std::move(entries));
            } else {
                current = saved; // restore — it's a block statement
            }
        } else if (check(TokenType::RIGHT_BRACE)) {
            // empty dict {}
            advance(); // consume }
            std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> entries;
            return std::make_unique<DictExpr>(std::move(entries));
        } else {
            current = saved; // restore
        }
    }

    throw std::runtime_error("Parser error at line " + std::to_string(peek().line) + ": unexpected token '" + peek().lexeme + "'");
}

// =================== Helpers ===================
bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) { advance(); return true; }
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

Token Parser::peek() const { return tokens[current]; }

Token Parser::peekNext() const {
    if (current + 1 >= (int)tokens.size()) return tokens.back();
    return tokens[current + 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error("Parser error: " + message);
}

bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }

// =================== Expression Evaluate Implementations ===================

void LiteralExpr::print() const { std::cout << value.lexeme; }
SatanValue LiteralExpr::evaluate(Environment& env) const {
    if (value.type == TokenType::NUMBER) return SatanValue(std::stod(value.lexeme));
    if (value.type == TokenType::STRING) return SatanValue(value.lexeme);
    if (value.type == TokenType::TRUE) return SatanValue(true);
    if (value.type == TokenType::FALSE) return SatanValue(false);
    return SatanValue();
}

void VariableExpr::print() const { std::cout << name.lexeme; }
SatanValue VariableExpr::evaluate(Environment& env) const {
    return env.get(name.lexeme);
}

void BinaryExpr::print() const {
    std::cout << "("; left->print(); std::cout << " " << op.lexeme << " "; right->print(); std::cout << ")";
}
SatanValue BinaryExpr::evaluate(Environment& env) const {
    SatanValue l = left->evaluate(env);
    SatanValue r = right->evaluate(env);
    switch (op.type) {
        case TokenType::PLUS:
            if (l.isNumber() && r.isNumber()) return SatanValue(l.number + r.number);
            return SatanValue(l.toString() + r.toString());
        case TokenType::MINUS: return SatanValue(l.asNumber() - r.asNumber());
        case TokenType::STAR:  return SatanValue(l.asNumber() * r.asNumber());
        case TokenType::SLASH:
            if (r.asNumber() == 0) throw std::runtime_error("Division by zero.");
            return SatanValue(l.asNumber() / r.asNumber());
        case TokenType::PERCENT:
            if (r.asNumber() == 0) throw std::runtime_error("Modulo by zero.");
            return SatanValue(std::fmod(l.asNumber(), r.asNumber()));
        case TokenType::GREATER:       return SatanValue(l.asNumber() > r.asNumber());
        case TokenType::GREATER_EQUAL: return SatanValue(l.asNumber() >= r.asNumber());
        case TokenType::LESS:          return SatanValue(l.asNumber() < r.asNumber());
        case TokenType::LESS_EQUAL:    return SatanValue(l.asNumber() <= r.asNumber());
        case TokenType::EQUAL_EQUAL:
            if (l.type != r.type) return SatanValue(false);
            if (l.isNumber()) return SatanValue(l.number == r.number);
            if (l.isString()) return SatanValue(l.str == r.str);
            if (l.isBoolean()) return SatanValue(l.boolean == r.boolean);
            return SatanValue(false);
        case TokenType::BANG_EQUAL:
            if (l.type != r.type) return SatanValue(true);
            if (l.isNumber()) return SatanValue(l.number != r.number);
            if (l.isString()) return SatanValue(l.str != r.str);
            if (l.isBoolean()) return SatanValue(l.boolean != r.boolean);
            return SatanValue(true);
        default:
            throw std::runtime_error("Unknown binary operator: " + op.lexeme);
    }
}

void CallExpr::print() const {
    callee->print(); std::cout << "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) std::cout << ", ";
        arguments[i]->print();
    }
    std::cout << ")";
}
SatanValue CallExpr::evaluate(Environment& env) const {
    SatanValue fn = callee->evaluate(env);

    // Native function
    if (fn.isNativeFn() && fn.nativeFn) {
        std::vector<SatanValue> args;
        for (const auto& arg : arguments) {
            args.push_back(arg->evaluate(env));
        }
        return (*fn.nativeFn)(args);
    }

    // User-defined function
    if (fn.isFunction() && fn.function) {
        const FunctionObject& func = *fn.function;
        if (arguments.size() != func.params.size()) {
            throw std::runtime_error("Expected " + std::to_string(func.params.size()) +
                                     " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        Environment callEnv(&env);
        for (size_t i = 0; i < func.params.size(); i++) {
            SatanValue argVal = arguments[i]->evaluate(env);
            callEnv.define(func.params[i], std::move(argVal));
        }
        try {
            func.body->execute(callEnv);
        } catch (const ReturnException& ret) {
            return ret.value;
        }
        return SatanValue();
    }

    throw std::runtime_error("Can only call functions.");
}

void LogicalExpr::print() const {
    std::cout << "Logical("; left->print(); std::cout << " " << op.lexeme << " "; right->print(); std::cout << ")";
}
SatanValue LogicalExpr::evaluate(Environment& env) const {
    SatanValue leftVal = left->evaluate(env);
    if (op.type == TokenType::OR) {
        if (leftVal.isTruthy()) return SatanValue(true);
        return SatanValue(right->evaluate(env).isTruthy());
    }
    if (op.type == TokenType::AND) {
        if (!leftVal.isTruthy()) return SatanValue(false);
        return SatanValue(right->evaluate(env).isTruthy());
    }
    return SatanValue(false);
}

void UnaryExpr::print() const {
    std::cout << "Unary(" << op.lexeme << " "; right->print(); std::cout << ")";
}
SatanValue UnaryExpr::evaluate(Environment& env) const {
    SatanValue val = right->evaluate(env);
    if (op.type == TokenType::MINUS) return SatanValue(-val.asNumber());
    if (op.type == TokenType::BANG) return SatanValue(!val.isTruthy());
    return val;
}

// NEW expression implementations
void ArrayExpr::print() const {
    std::cout << "[";
    for (size_t i = 0; i < elements.size(); i++) {
        if (i > 0) std::cout << ", ";
        elements[i]->print();
    }
    std::cout << "]";
}
SatanValue ArrayExpr::evaluate(Environment& env) const {
    std::vector<SatanValue> vals;
    for (const auto& elem : elements) vals.push_back(elem->evaluate(env));
    return SatanValue::makeArray(std::move(vals));
}

void MemberAccessExpr::print() const {
    object->print(); std::cout << "." << member.lexeme;
}
SatanValue MemberAccessExpr::evaluate(Environment& env) const {
    SatanValue obj = object->evaluate(env);
    // For ML objects, use the property handler
    if (obj.isObject()) {
        auto typeIt = obj.object->find("__type__");
        if (typeIt != obj.object->end() && Interpreter::current) {
            return handlePropertyAccess(obj, member.lexeme, Interpreter::current->getBridge());
        }
        return obj.getProperty(member.lexeme);
    }
    if (obj.isArray() && member.lexeme == "length") {
        return SatanValue(static_cast<double>(obj.array ? obj.array->size() : 0));
    }
    if (obj.isString() && member.lexeme == "length") {
        return SatanValue(static_cast<double>(obj.str.size()));
    }
    return obj.getProperty(member.lexeme);
}

void MethodCallExpr::print() const {
    object->print(); std::cout << "." << method.lexeme << "(";
    for (size_t i = 0; i < arguments.size(); i++) {
        if (i > 0) std::cout << ", ";
        arguments[i]->print();
    }
    std::cout << ")";
}
SatanValue MethodCallExpr::evaluate(Environment& env) const {
    SatanValue obj = object->evaluate(env);
    std::vector<SatanValue> args;
    for (const auto& arg : arguments) args.push_back(arg->evaluate(env));

    // Array built-in methods
    if (obj.isArray()) {
        if (method.lexeme == "push" && args.size() == 1) {
            obj.array->push_back(args[0]);
            return SatanValue();
        }
        if (method.lexeme == "pop" && obj.array && !obj.array->empty()) {
            SatanValue last = obj.array->back();
            obj.array->pop_back();
            return last;
        }
        if (method.lexeme == "size" || method.lexeme == "length") {
            return SatanValue(static_cast<double>(obj.array ? obj.array->size() : 0));
        }
        if (method.lexeme == "map" && args.size() == 1 && args[0].isCallable()) {
            std::vector<SatanValue> result;
            for (const auto& elem : *obj.array) {
                std::vector<SatanValue> fnArgs = {elem};
                if (args[0].isNativeFn()) result.push_back((*args[0].nativeFn)(fnArgs));
                else if (args[0].isFunction()) {
                    Environment callEnv(&env);
                    for (size_t i = 0; i < args[0].function->params.size() && i < fnArgs.size(); i++)
                        callEnv.define(args[0].function->params[i], fnArgs[i]);
                    try { args[0].function->body->execute(callEnv); result.push_back(SatanValue()); }
                    catch (const ReturnException& ret) { result.push_back(ret.value); }
                }
            }
            return SatanValue::makeArray(std::move(result));
        }
        if (method.lexeme == "filter" && args.size() == 1 && args[0].isCallable()) {
            std::vector<SatanValue> result;
            for (const auto& elem : *obj.array) {
                std::vector<SatanValue> fnArgs = {elem};
                SatanValue res;
                if (args[0].isNativeFn()) res = (*args[0].nativeFn)(fnArgs);
                else if (args[0].isFunction()) {
                    Environment callEnv(&env);
                    for (size_t i = 0; i < args[0].function->params.size() && i < fnArgs.size(); i++)
                        callEnv.define(args[0].function->params[i], fnArgs[i]);
                    try { args[0].function->body->execute(callEnv); }
                    catch (const ReturnException& ret) { res = ret.value; }
                }
                if (res.isTruthy()) result.push_back(elem);
            }
            return SatanValue::makeArray(std::move(result));
        }
        if (method.lexeme == "forEach" && args.size() == 1 && args[0].isCallable()) {
            for (const auto& elem : *obj.array) {
                std::vector<SatanValue> fnArgs = {elem};
                if (args[0].isNativeFn()) (*args[0].nativeFn)(fnArgs);
                else if (args[0].isFunction()) {
                    Environment callEnv(&env);
                    for (size_t i = 0; i < args[0].function->params.size() && i < fnArgs.size(); i++)
                        callEnv.define(args[0].function->params[i], fnArgs[i]);
                    try { args[0].function->body->execute(callEnv); }
                    catch (const ReturnException&) {}
                }
            }
            return SatanValue();
        }
        if (method.lexeme == "join") {
            std::string delim = args.empty() ? ", " : args[0].toString();
            std::string result;
            for (size_t i = 0; i < obj.array->size(); i++) {
                if (i > 0) result += delim;
                result += (*obj.array)[i].toString();
            }
            return SatanValue(result);
        }
        if (method.lexeme == "indexOf" && args.size() == 1) {
            for (size_t i = 0; i < obj.array->size(); i++) {
                auto& el = (*obj.array)[i];
                if (el.type == args[0].type) {
                    if ((el.isNumber() && el.number == args[0].number) ||
                        (el.isString() && el.str == args[0].str))
                        return SatanValue(static_cast<double>(i));
                }
            }
            return SatanValue(-1.0);
        }
        if (method.lexeme == "contains" && args.size() == 1) {
            for (const auto& el : *obj.array) {
                if (el.type == args[0].type) {
                    if ((el.isNumber() && el.number == args[0].number) ||
                        (el.isString() && el.str == args[0].str))
                        return SatanValue(true);
                }
            }
            return SatanValue(false);
        }
        if (method.lexeme == "reverse") {
            std::vector<SatanValue> rev(obj.array->rbegin(), obj.array->rend());
            return SatanValue::makeArray(std::move(rev));
        }
        if (method.lexeme == "slice") {
            int start = args.size() > 0 ? (int)args[0].asNumber() : 0;
            int end = args.size() > 1 ? (int)args[1].asNumber() : (int)obj.array->size();
            if (start < 0) start = 0;
            if (end > (int)obj.array->size()) end = (int)obj.array->size();
            std::vector<SatanValue> sliced(obj.array->begin() + start, obj.array->begin() + end);
            return SatanValue::makeArray(std::move(sliced));
        }
        if (method.lexeme == "sort") {
            std::vector<SatanValue> sorted = *obj.array;
            std::sort(sorted.begin(), sorted.end(), [](const SatanValue& a, const SatanValue& b) {
                if (a.isNumber() && b.isNumber()) return a.number < b.number;
                return a.toString() < b.toString();
            });
            return SatanValue::makeArray(std::move(sorted));
        }
    }

    // String built-in methods
    if (obj.isString()) {
        if (method.lexeme == "upper") {
            std::string s = obj.str;
            for (auto& c : s) c = toupper(c);
            return SatanValue(s);
        }
        if (method.lexeme == "lower") {
            std::string s = obj.str;
            for (auto& c : s) c = tolower(c);
            return SatanValue(s);
        }
        if (method.lexeme == "split") {
            std::string delim = args.empty() ? " " : args[0].toString();
            std::vector<SatanValue> parts;
            size_t pos = 0; std::string s = obj.str;
            while ((pos = s.find(delim)) != std::string::npos) {
                parts.push_back(SatanValue(s.substr(0, pos)));
                s.erase(0, pos + delim.length());
            }
            parts.push_back(SatanValue(s));
            return SatanValue::makeArray(std::move(parts));
        }
        if (method.lexeme == "trim") {
            std::string s = obj.str;
            s.erase(0, s.find_first_not_of(" \t\n\r"));
            s.erase(s.find_last_not_of(" \t\n\r") + 1);
            return SatanValue(s);
        }
        if (method.lexeme == "replace") {
            if (args.size() < 2) throw std::runtime_error("replace() requires 2 arguments.");
            std::string s = obj.str;
            std::string from = args[0].toString(), to = args[1].toString();
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos) {
                s.replace(pos, from.length(), to);
                pos += to.length();
            }
            return SatanValue(s);
        }
        if (method.lexeme == "starts_with" && args.size() == 1) {
            return SatanValue(obj.str.substr(0, args[0].str.size()) == args[0].str);
        }
        if (method.lexeme == "ends_with" && args.size() == 1) {
            if (args[0].str.size() > obj.str.size()) return SatanValue(false);
            return SatanValue(obj.str.substr(obj.str.size() - args[0].str.size()) == args[0].str);
        }
        if (method.lexeme == "charAt" && args.size() == 1) {
            int idx = (int)args[0].asNumber();
            if (idx >= 0 && idx < (int)obj.str.size()) return SatanValue(std::string(1, obj.str[idx]));
            return SatanValue(std::string(""));
        }
        if (method.lexeme == "includes" || method.lexeme == "contains") {
            return SatanValue(obj.str.find(args[0].toString()) != std::string::npos);
        }
        if (method.lexeme == "substring") {
            int start = args.size() > 0 ? (int)args[0].asNumber() : 0;
            int len = args.size() > 1 ? (int)args[1].asNumber() : (int)obj.str.size() - start;
            return SatanValue(obj.str.substr(start, len));
        }
        if (method.lexeme == "repeat") {
            int count = args.size() > 0 ? (int)args[0].asNumber() : 1;
            std::string result;
            for (int i = 0; i < count; i++) result += obj.str;
            return SatanValue(result);
        }
    }

    // Object/Dictionary built-in methods
    if (obj.isObject() && obj.object) {
        if (method.lexeme == "keys") {
            std::vector<SatanValue> keys;
            for (const auto& p : *obj.object) {
                if (p.first.size() >= 2 && p.first[0] == '_' && p.first[1] == '_') continue;
                keys.push_back(SatanValue(p.first));
            }
            return SatanValue::makeArray(std::move(keys));
        }
        if (method.lexeme == "values") {
            std::vector<SatanValue> vals;
            for (const auto& p : *obj.object) {
                if (p.first.size() >= 2 && p.first[0] == '_' && p.first[1] == '_') continue;
                vals.push_back(p.second);
            }
            return SatanValue::makeArray(std::move(vals));
        }
        if (method.lexeme == "has" && args.size() == 1) {
            return SatanValue(obj.object->count(args[0].toString()) > 0);
        }
        if (method.lexeme == "size") {
            int count = 0;
            for (const auto& p : *obj.object)
                if (!(p.first.size() >= 2 && p.first[0] == '_' && p.first[1] == '_')) count++;
            return SatanValue(static_cast<double>(count));
        }
    }

    // For ML objects, delegate to the ML handler
    if (obj.isObject() && Interpreter::current) {
        return handleMethodCall(obj, method.lexeme, args, Interpreter::current->getBridge());
    }

    throw std::runtime_error("Cannot call method '" + method.lexeme + "' on " + obj.toString());
}

void IndexExpr::print() const {
    object->print(); std::cout << "["; index->print(); std::cout << "]";
}
SatanValue IndexExpr::evaluate(Environment& env) const {
    SatanValue obj = object->evaluate(env);
    SatanValue idx = index->evaluate(env);
    if (obj.isArray() && idx.isNumber()) {
        int i = static_cast<int>(idx.number);
        if (i < 0 || i >= (int)obj.array->size())
            throw std::runtime_error("Array index out of bounds: " + std::to_string(i));
        return (*obj.array)[i];
    }
    if (obj.isString() && idx.isNumber()) {
        int i = static_cast<int>(idx.number);
        if (i < 0 || i >= (int)obj.str.size())
            throw std::runtime_error("String index out of bounds: " + std::to_string(i));
        return SatanValue(std::string(1, obj.str[i]));
    }
    if (obj.isObject() && idx.isString()) {
        return obj.getProperty(idx.str);
    }
    throw std::runtime_error("Cannot index " + obj.toString());
}

void AssignExpr::print() const {
    std::cout << name.lexeme << " = "; value->print();
}
SatanValue AssignExpr::evaluate(Environment& env) const {
    SatanValue val = value->evaluate(env);
    env.assign(name.lexeme, val);
    return val;
}

void NamedArgExpr::print() const {
    std::cout << name.lexeme << "="; value->print();
}
SatanValue NamedArgExpr::evaluate(Environment& env) const {
    // Named args are handled by the caller (CallExpr/MethodCallExpr)
    return value->evaluate(env);
}

// =================== Statement Implementations ===================

void VarDecl::execute(Environment& env) const {
    SatanValue val;
    if (initializer) val = initializer->evaluate(env);
    env.define(name.lexeme, std::move(val));
}

void AssembleStmt::execute(Environment& env) const {
    SatanValue val = expr->evaluate(env);
    std::cout << val.toString() << std::endl;
}

void PrintStmt::execute(Environment& env) const {
    SatanValue val = expr->evaluate(env);
    std::cout << val.toString() << std::endl;
}

void IfStmt::execute(Environment& env) const {
    SatanValue condVal = condition->evaluate(env);
    if (condVal.isTruthy()) {
        thenBranch->execute(env);
    } else if (elseBranch) {
        elseBranch->execute(env);
    }
}

void BlockStmt::execute(Environment& env) const {
    Environment blockEnv(&env);
    for (const auto& stmt : statements) stmt->execute(blockEnv);
}

void ExprStmt::execute(Environment& env) const {
    expr->evaluate(env);
}

void SummonStmt::execute(Environment& env) const {
    SatanValue val = message->evaluate(env);
    std::cout << val.toString() << std::endl;
}

void FunDecl::execute(Environment& env) const {
    FunctionObject func;
    for (const auto& param : params) func.params.push_back(param.lexeme);
    func.body = body;
    env.defineFunction(name.lexeme, func);
}

void ReturnStmt::execute(Environment& env) const {
    SatanValue val;
    if (value) val = value->evaluate(env);
    throw ReturnException(std::move(val));
}

void WhileStmt::execute(Environment& env) const {
    int iterations = 0;
    while (condition->evaluate(env).isTruthy()) {
        if (++iterations > MAX_LOOP_ITERATIONS)
            throw std::runtime_error("While loop exceeded maximum iteration limit");
        try { body->execute(env); }
        catch (const BreakSignal&) { break; }
        catch (const ContinueSignal&) { continue; }
    }
}

void ForStmt::execute(Environment& env) const {
    if (initializer) initializer->execute(env);
    int iterations = 0;
    while (!condition || condition->evaluate(env).isTruthy()) {
        if (++iterations > MAX_LOOP_ITERATIONS)
            throw std::runtime_error("For loop exceeded maximum iteration limit");
        try { body->execute(env); }
        catch (const BreakSignal&) { break; }
        catch (const ContinueSignal&) { if (increment) increment->evaluate(env); continue; }
        if (increment) increment->evaluate(env);
    }
}

void BreakStmt::execute(Environment&) const { throw BreakSignal(); }
void ContinueStmt::execute(Environment&) const { throw ContinueSignal(); }

// =================== Phase 1: Parsing Methods ===================

std::unique_ptr<Stmt> Parser::tryCatchStatement() {
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'try'.");
    auto tryBlock = parseBlock();
    consume(TokenType::CATCH, "Expect 'catch' after try block.");
    Token catchVar(TokenType::IDENTIFIER, "_err", 0);
    bool hasCatchVar = false;
    if (match({TokenType::LEFT_PAREN})) {
        catchVar = consume(TokenType::IDENTIFIER, "Expect error variable name.");
        consume(TokenType::RIGHT_PAREN, "Expect ')' after catch variable.");
        hasCatchVar = true;
    }
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'catch'.");
    auto catchBlock = parseBlock();
    return std::make_unique<TryCatchStmt>(std::move(tryBlock), std::move(catchBlock), catchVar, hasCatchVar);
}

std::unique_ptr<Stmt> Parser::importStatement() {
    Token path = consume(TokenType::STRING, "Expect file path string after 'import'.");
    consume(TokenType::SEMICOLON, "Expect ';' after import path.");
    return std::make_unique<ImportStmt>(path.lexeme);
}

std::unique_ptr<Stmt> Parser::forInOrForStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    // Check for 'for (let x in expr)' pattern
    if (check(TokenType::LET) || check(TokenType::VAR)) {
        int saved = current;
        advance(); // skip let/var
        if (check(TokenType::IDENTIFIER)) {
            Token varName = advance();
            if (check(TokenType::IN)) {
                advance(); // skip 'in'
                auto iterable = expression();
                consume(TokenType::RIGHT_PAREN, "Expect ')' after for..in.");
                auto body = statement();
                return std::make_unique<ForInStmt>(varName, std::move(iterable), std::move(body));
            }
        }
        current = saved; // not a for..in, restore and parse as normal for
    }
    // Regular C-style for loop
    std::unique_ptr<Stmt> initializer;
    if (match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if (match({TokenType::VAR})) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }
    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) condition = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) increment = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
    auto body = statement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::assertStatement() {
    auto condition = expression();
    std::string message = "Assertion failed";
    if (match({TokenType::COMMA})) {
        Token msg = consume(TokenType::STRING, "Expect error message string after ','.");
        message = msg.lexeme;
    }
    consume(TokenType::SEMICOLON, "Expect ';' after assert.");
    return std::make_unique<AssertStmt>(std::move(condition), message);
}

std::unique_ptr<Stmt> Parser::testStatement() {
    Token name = consume(TokenType::STRING, "Expect test name string after 'test'.");
    consume(TokenType::LEFT_BRACE, "Expect '{' after test name.");
    auto body = parseBlock();
    return std::make_unique<TestStmt>(name.lexeme, std::move(body));
}

// =================== Phase 1: Execution Implementations ===================

void TryCatchStmt::execute(Environment& env) const {
    try {
        tryBlock->execute(env);
    } catch (const ReturnException&) {
        throw; // Don't catch returns
    } catch (const BreakSignal&) {
        throw; // Don't catch breaks
    } catch (const ContinueSignal&) {
        throw; // Don't catch continues
    } catch (const std::exception& e) {
        Environment catchEnv(&env);
        if (hasCatchVar) {
            catchEnv.define(catchVar.lexeme, SatanValue(std::string(e.what())));
        }
        catchBlock->execute(catchEnv);
    }
}

void ImportStmt::execute(Environment& env) const {
    // Resolve path relative to current working directory
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot import '" + filepath + "': file not found.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(tokens);
    auto stmts = parser.parse();

    for (const auto& stmt : stmts) {
        stmt->execute(env);
    }
}

void ForInStmt::execute(Environment& env) const {
    SatanValue iterVal = iterable->evaluate(env);
    int iterations = 0;
    if (iterVal.isArray() && iterVal.array) {
        for (const auto& elem : *iterVal.array) {
            if (++iterations > 1000000) throw std::runtime_error("For..in loop exceeded max iterations");
            Environment loopEnv(&env);
            loopEnv.define(varName.lexeme, elem);
            try { body->execute(loopEnv); }
            catch (const BreakSignal&) { break; }
            catch (const ContinueSignal&) { continue; }
        }
    } else if (iterVal.isString()) {
        for (char c : iterVal.str) {
            if (++iterations > 1000000) throw std::runtime_error("For..in loop exceeded max iterations");
            Environment loopEnv(&env);
            loopEnv.define(varName.lexeme, SatanValue(std::string(1, c)));
            try { body->execute(loopEnv); }
            catch (const BreakSignal&) { break; }
            catch (const ContinueSignal&) { continue; }
        }
    } else if (iterVal.isObject() && iterVal.object) {
        for (const auto& pair : *iterVal.object) {
            if (pair.first[0] == '_' && pair.first[1] == '_') continue; // skip internal props
            if (++iterations > 1000000) throw std::runtime_error("For..in loop exceeded max iterations");
            Environment loopEnv(&env);
            loopEnv.define(varName.lexeme, SatanValue(pair.first));
            try { body->execute(loopEnv); }
            catch (const BreakSignal&) { break; }
            catch (const ContinueSignal&) { continue; }
        }
    } else {
        throw std::runtime_error("Cannot iterate over " + iterVal.toString());
    }
}

void AssertStmt::execute(Environment& env) const {
    SatanValue val = condition->evaluate(env);
    if (!val.isTruthy()) {
        throw std::runtime_error("Assertion failed: " + message);
    }
}

void TestStmt::execute(Environment& env) const {
    std::cout << "\033[36m  TEST: " << name << "...\033[0m ";
    try {
        Environment testEnv(&env);
        body->execute(testEnv);
        std::cout << "\033[32mPASSED\033[0m" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "\033[31mFAILED: " << e.what() << "\033[0m" << std::endl;
    }
}

void DictExpr::print() const {
    std::cout << "{";
    for (size_t i = 0; i < entries.size(); i++) {
        if (i > 0) std::cout << ", ";
        entries[i].first->print();
        std::cout << ": ";
        entries[i].second->print();
    }
    std::cout << "}";
}

SatanValue DictExpr::evaluate(Environment& env) const {
    SatanValue obj = SatanValue::makeObject();
    for (const auto& entry : entries) {
        SatanValue key = entry.first->evaluate(env);
        SatanValue val = entry.second->evaluate(env);
        obj.setProperty(key.toString(), std::move(val));
    }
    return obj;
}
