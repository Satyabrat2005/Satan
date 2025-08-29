#include "../include/interpreter.h"
#include <iostream>

void Interpreter::interpret(const Expr* expr) const {
    evaluate(expr);
}

void Interpreter::evaluate(const Expr* expr) const {
    expr->print();  //Fr now am printing AST only !!
}
