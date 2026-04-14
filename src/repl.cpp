#include "../include/repl.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include <iostream>
#include <string>
#include <sstream>

Repl::Repl() : interpreter() {}

void Repl::run() {
    std::cout << "Satan REPL v0.1.0" << std::endl;
    std::cout << "Type 'exit' or 'quit' to leave the REPL." << std::endl;
    std::cout << std::endl;

    std::string line;

    while (true) {
        std::cout << "satan> ";
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            std::cout << std::endl;
            break;
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue; // empty line
        }
        std::string trimmed = line.substr(start);
        size_t end = trimmed.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            trimmed = trimmed.substr(0, end + 1);
        }

        if (trimmed == "exit" || trimmed == "quit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }

        if (trimmed.empty()) {
            continue;
        }

        try {
            // Lex
            Lexer lexer(line);
            auto tokens = lexer.scanTokens();

            // Parse
            Parser parser(tokens);
            auto statements = parser.parse();

            if (statements.empty()) {
                std::cerr << "[error] Could not parse input." << std::endl;
                continue;
            }

            // Execute
            interpreter.interpret(statements);

        } catch (const std::runtime_error& e) {
            std::cerr << "[error] " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[error] " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[error] An unknown error occurred." << std::endl;
        }
    }
}
