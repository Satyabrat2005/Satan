#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include "environment.h"
#include "python_bridge.h"
#include <memory>
#include <vector>
#include <stdexcept>

class ReturnException : public std::runtime_error {
public:
    SatanValue value;
    explicit ReturnException(SatanValue val)
        : std::runtime_error("Function returned"), value(std::move(val)) {}
};

class Interpreter {
public:
    Interpreter();

    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);
    void registerBuiltins();

    Environment& getEnv() { return env; }
    PythonBridge& getBridge() { return bridge; }

    // Static access for ML evaluations
    static Interpreter* current;

private:
    Environment env;
    PythonBridge bridge;

    void execute(const Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment& newEnv);
    SatanValue evaluate(const Expr* expr);
};

#endif
