#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/environment.h"
#include <iostream>

Environment globalEnv;

int main() {
    std::string source = R"(
        let x = 10 + 2 * 3;
        let y = x - 4;
        print x;
        print y;
    )";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    // Execute each statement with environment
    for (const auto& stmt : statements) {
        stmt->execute(globalEnv);
    }

    return 0;
}
