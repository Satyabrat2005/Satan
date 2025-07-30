#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "environment.h"
#include <memory>
#include <vector>

// ---- Expression Base ----
class Expr {
public:
    virtual ~Expr() = default;
    virtual void print() const = 0;
    virtual double evaluate(Environment& env) const = 0;
};

// ---- Expression Nodes ----
class LiteralExpr : public Expr {
public:
    Token value;
    explicit LiteralExpr(Token val) : value(std::move(val)) {}
    void print() const override;
    double evaluate(Environment& env) const override;
};

class VariableExpr : public Expr {
public:
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
    void print() const override;
    double evaluate(Environment& env) const override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    void print() const override;
    double evaluate(Environment& env) const override;
};

// ---- Statement Base ----
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void execute(Environment& env) const = 0;
};

// ---- Statement Nodes ----
class VarDecl : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> initializer;

    VarDecl(Token n, std::unique_ptr<Expr> init)
        : name(std::move(n)), initializer(std::move(init)) {}

    void execute(Environment& env) const override;
};

class AssembleStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit AssembleStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void execute(Environment& env) const override;
};


class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    explicit PrintStmt(std::unique_ptr<Expr> e)
        : expr(std::move(e)) {}

    void execute(Environment& env) const override;
};

class AssembleStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit AssembleStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void execute(Environment& env) const override;
};

// ---- Parser ----
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current;

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> assembleStatement();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> primary();

    Token advance();
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token peek() const;
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;
};

#endif // PARSER_H
