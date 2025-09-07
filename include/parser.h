#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "lexer.h"
#include "environment.h"
#include <memory>
#include <vector>

class Expr {
public:
    virtual ~Expr() = default;
    virtual void print() const = 0;
    virtual double evaluate(Environment& env) const = 0;
};

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


class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}

    void print() const override;
    double evaluate(Environment& env) const override;
};


class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void execute(Environment& env) const = 0;
};

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

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;

    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenB, std::unique_ptr<Stmt> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}

    void execute(Environment& env) const override;
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}

    void execute(Environment& env) const override;
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void execute(Environment& env) const override;
};

class SummonStmt : public Stmt {
public:
    explicit SummonStmt(std::unique_ptr<Expr> msg) : message(std::move(msg)) {}
    void execute(Environment& env) const override;

private:
    std::unique_ptr<Expr> message;
};


class FunDecl : public Stmt {
public:
    Token name;
    std::vector<Token> params;
    std::unique_ptr<BlockStmt> body;

    FunDecl(Token n, std::vector<Token> p, std::unique_ptr<BlockStmt> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}

    void execute(Environment& env) const override;
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;

    ReturnStmt(Token k, std::unique_ptr<Expr> v)
        : keyword(std::move(k)), value(std::move(v)) {}

    void execute(Environment& env) const override;
};


class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current;

    // ---- Statements ----
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> funDeclaration();   
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> assembleStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> summonStatement();
    std::unique_ptr<Stmt> returnStatement();  
    std::unique_ptr<Stmt> parseBlock();
    std::unique_ptr<Stmt> expressionStatement();

    // ---- Expressions ----
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();          
    std::unique_ptr<Expr> call();             
    std::unique_ptr<Expr> primary();

    // ---- Helpers ----
    Token advance();
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token peek() const;
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;
};

#endif
