#include "../include/interpreter.h"
#include "../include/stdlib_ml.h"
#include <iostream>

Interpreter* Interpreter::current = nullptr;

Interpreter::Interpreter() : env(), bridge() {
    current = this;
    bridge.initSession();
    registerBuiltins();
}

void Interpreter::registerBuiltins() {
    registerMLBuiltins(env, bridge);
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    current = this;
    try {
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
    } catch (const BreakSignal&) {
        std::cerr << "[runtime error] 'break' used outside of a loop" << std::endl;
    } catch (const ContinueSignal&) {
        std::cerr << "[runtime error] 'continue' used outside of a loop" << std::endl;
    } catch (const std::runtime_error& err) {
        std::cerr << "[runtime error] " << err.what() << std::endl;
    }
}

void Interpreter::execute(const Stmt* stmt) {
    if (!stmt) return;
    stmt->execute(env);
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, Environment& newEnv) {
    for (const auto& stmt : statements) {
        stmt->execute(newEnv);
    }
}

SatanValue Interpreter::evaluate(const Expr* expr) {
    if (!expr) return SatanValue();
    return expr->evaluate(env);
}
