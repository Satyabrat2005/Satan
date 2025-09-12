#include "../include/interpreter.h"
#include <iostream>

// ---------------- Constructor ----------------
Interpreter::Interpreter() : env() {}

// ---------------- Program Runner ----------------
void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
    } catch (const std::runtime_error& err) {
        std::cerr << "[runtime error] " << err.what() << std::endl;
    }
}

// ---------------- Statement Execution ----------------
void Interpreter::execute(const Stmt* stmt) {
    if (!stmt) return;
    stmt->execute(env);
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment& newEnv) {
    for (const auto& stmt : statements) {
        stmt->execute(newEnv);
    }
}

// ---------------- Expression Evaluation ----------------
double Interpreter::evaluate(const Expr* expr) {
    if (!expr) return 0;
    return expr->evaluate(env);
}
