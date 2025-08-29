#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

class Interpreter {
public:
    void interpret(const Expr* expr) const;
private:
    void evaluate(const Expr* expr) const;
};

#endif
