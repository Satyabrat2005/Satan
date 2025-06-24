#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>

// ---- AST Node Types ----

class Expr {
public:
    virtual ~Expr() = default;
    virtual void print() const = 0;
};

class LiteralExpr : public Expr {
public:
    Token value;
    explicit LiteralExpr(Token val) : value(std::move(val)) {}
    void print() const override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    void print() const override;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Expr> parse();

private:
    const std::vector<Token>& tokens;
    int current;

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> primary();

    Token advance();
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token peek() const;
    bool isAtEnd() const;
};

#endif // PARSER_H
