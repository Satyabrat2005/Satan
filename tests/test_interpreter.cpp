#include "../include/lexer.h"
#include "../include/parser.h"
#include <iostream>

int main() {
    std::string source = "let x = 5 + 3 * 2; print x;";
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    for (const auto& stmt : statements) {
        stmt->execute();
    }

    return 0;
}
