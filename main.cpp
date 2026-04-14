#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/interpreter.h"
#include "include/repl.h"

static void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    Parser parser(tokens);
    auto statements = parser.parse();

    if (statements.empty()) {
        std::cerr << "Parsing failed." << std::endl;
        return;
    }

    Interpreter interpreter;
    interpreter.interpret(statements);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        // No file argument — launch the REPL
        Repl repl;
        repl.run();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: satan [script.satan]" << std::endl;
        return 1;
    }
    return 0;
}
