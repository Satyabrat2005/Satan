#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/interpreter.h"
#include "include/repl.h"
#include "include/setup.h"

static void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "\033[31m[error]\033[0m Could not open file '" << path << "'" << std::endl;
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
        std::cerr << "\033[31m[error]\033[0m Parsing failed." << std::endl;
        return;
    }

    Interpreter interpreter;
    interpreter.interpret(statements);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Repl repl;
        repl.run();
    } else if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v") {
        printVersion();
    } else if (std::string(argv[1]) == "--check") {
        checkDependencies();
    } else if (std::string(argv[1]) == "--setup-ml") {
        installMLDependencies();
    } else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        printVersion();
        std::cout << " Usage:" << std::endl;
        std::cout << "   satan                   Launch REPL" << std::endl;
        std::cout << "   satan <script.satan>     Run a Satan script" << std::endl;
        std::cout << "   satan --version          Show version info" << std::endl;
        std::cout << "   satan --check            Check dependencies" << std::endl;
        std::cout << "   satan --setup-ml         Install ML dependencies" << std::endl;
        std::cout << "   satan --help             Show this help" << std::endl;
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: satan [script.satan]" << std::endl;
        return 1;
    }
    return 0;
}
