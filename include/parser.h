#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "lexer.h"
#include "environment.h"
#include "satan_value.h"
#include <memory>
#include <vector>
#include <optional>

constexpr int MAX_PARSE_DEPTH = 256;

class BreakSignal : public std::exception {
public:
    const char* what() const noexcept override { return "break"; }
};

class ContinueSignal : public std::exception {
public:
    const char* what() const noexcept override { return "continue"; }
};

// =================== Expressions ===================
class Expr {
public:
    virtual ~Expr() = default;
    virtual void print() const = 0;
    virtual SatanValue evaluate(Environment& env) const = 0;
};

class LiteralExpr : public Expr {
public:
    Token value;
    explicit LiteralExpr(Token val) : value(std::move(val)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

class VariableExpr : public Expr {
public:
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

class LogicalExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token o, std::unique_ptr<Expr> r)
        : op(std::move(o)), right(std::move(r)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Array literal [1, 2, 3]
class ArrayExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elems)
        : elements(std::move(elems)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Member access: obj.property
class MemberAccessExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token member;
    MemberAccessExpr(std::unique_ptr<Expr> obj, Token m)
        : object(std::move(obj)), member(std::move(m)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Method call: obj.method(args)
class MethodCallExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token method;
    std::vector<std::unique_ptr<Expr>> arguments;
    MethodCallExpr(std::unique_ptr<Expr> obj, Token m, std::vector<std::unique_ptr<Expr>> args)
        : object(std::move(obj)), method(std::move(m)), arguments(std::move(args)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Index access: arr[0]
class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx)
        : object(std::move(obj)), index(std::move(idx)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Assignment: x = value
class AssignExpr : public Expr {
public:
    Token name;
    std::unique_ptr<Expr> value;
    AssignExpr(Token n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// NEW: Named argument: key=value (for function calls)
class NamedArgExpr : public Expr {
public:
    Token name;
    std::unique_ptr<Expr> value;
    NamedArgExpr(Token n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// =================== Statements ===================
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
    explicit PrintStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
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
    std::shared_ptr<BlockStmt> body;
    FunDecl(Token n, std::vector<Token> p, std::shared_ptr<BlockStmt> b)
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

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void execute(Environment& env) const override;
};

class ForStmt : public Stmt {
public:
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;
    ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> cond,
            std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> b)
        : initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(inc)), body(std::move(b)) {}
    void execute(Environment& env) const override;
};

class BreakStmt : public Stmt {
public:
    void execute(Environment& env) const override;
};

class ContinueStmt : public Stmt {
public:
    void execute(Environment& env) const override;
};

// Phase 1: try/catch
class TryCatchStmt : public Stmt {
public:
    std::unique_ptr<Stmt> tryBlock;
    std::unique_ptr<Stmt> catchBlock;
    Token catchVar; // optional error variable name
    bool hasCatchVar;
    TryCatchStmt(std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> c, Token cv, bool hcv)
        : tryBlock(std::move(t)), catchBlock(std::move(c)), catchVar(std::move(cv)), hasCatchVar(hcv) {}
    void execute(Environment& env) const override;
};

// Phase 1: import
class ImportStmt : public Stmt {
public:
    std::string filepath;
    explicit ImportStmt(std::string path) : filepath(std::move(path)) {}
    void execute(Environment& env) const override;
};

// Phase 1: for..in
class ForInStmt : public Stmt {
public:
    Token varName;
    std::unique_ptr<Expr> iterable;
    std::unique_ptr<Stmt> body;
    ForInStmt(Token v, std::unique_ptr<Expr> iter, std::unique_ptr<Stmt> b)
        : varName(std::move(v)), iterable(std::move(iter)), body(std::move(b)) {}
    void execute(Environment& env) const override;
};

// Phase 1: assert
class AssertStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::string message;
    AssertStmt(std::unique_ptr<Expr> cond, std::string msg)
        : condition(std::move(cond)), message(std::move(msg)) {}
    void execute(Environment& env) const override;
};

// Phase 1: test blocks
class TestStmt : public Stmt {
public:
    std::string name;
    std::unique_ptr<Stmt> body;
    TestStmt(std::string n, std::unique_ptr<Stmt> b)
        : name(std::move(n)), body(std::move(b)) {}
    void execute(Environment& env) const override;
};

// Phase 1: dictionary expression {key: value}
class DictExpr : public Expr {
public:
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> entries;
    explicit DictExpr(std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> e)
        : entries(std::move(e)) {}
    void print() const override;
    SatanValue evaluate(Environment& env) const override;
};

// =================== Parser ===================
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current;
    int depth = 0;

    struct DepthGuard {
        int& depth;
        explicit DepthGuard(int& d) : depth(d) {
            if (++depth > MAX_PARSE_DEPTH)
                throw std::runtime_error("Parser error: Maximum nesting depth exceeded.");
        }
        ~DepthGuard() { --depth; }
    };

    // Statements
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> funDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> assembleStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> forStatement();
    std::unique_ptr<Stmt> summonStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> breakStatement();
    std::unique_ptr<Stmt> continueStatement();
    std::unique_ptr<Stmt> parseBlock();
    std::unique_ptr<Stmt> expressionStatement();

    // Expressions
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logical();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();

    // Helpers
    Token advance();
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token peek() const;
    Token peekNext() const;
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;

    // Phase 1 parsers
    std::unique_ptr<Stmt> tryCatchStatement();
    std::unique_ptr<Stmt> importStatement();
    std::unique_ptr<Stmt> forInOrForStatement();
    std::unique_ptr<Stmt> assertStatement();
    std::unique_ptr<Stmt> testStatement();
};

#endif
