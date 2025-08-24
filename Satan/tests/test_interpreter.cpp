#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/environment.h"
#include <fstream>
#include <sstream>
#include <iostream>

int main() {
    std::ifstream input("tests/test_if.satan");
    if (!input.is_open()) {
        std::cerr << "Failed to open test_if.satan\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.scanTokens();

    for (const auto& token : tokens) {
    std::cout << "Token: " << static_cast<int>(token.type) << " — '" << token.lexeme << "'\n";
}

    Parser parser(tokens);
    auto statements = parser.parse();

    Environment env;
    for (const auto& stmt : statements) {
        stmt->execute(env);
    }

    return 0;
}
