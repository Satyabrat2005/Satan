#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "environment.h"
#include <memory>
#include <vector>
#include <stdexcept>

// Special exception for returning values from functions
class ReturnException : public std::runtime_error {
public:
    double value;
    explicit ReturnException(double val)
        : std::runtime_error("Function returned"), value(val) {}
};

class Interpreter {
public:
    Interpreter();

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

    Environment& getEnv() { return env; }

private:
    Environment env;

    // Execute statements
    void execute(const Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment& newEnv);

    // Evaluate expressions
    double evaluate(const Expr* expr);
};

#endif
