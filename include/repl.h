#ifndef REPL_H
#define REPL_H

#include "interpreter.h"

class Repl {
public:
    Repl();
    void run();

private:
    Interpreter interpreter;
};

#endif
