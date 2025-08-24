#include "../include/lexer.h"
#include "../include/parser.h"
#include <iostream>
#include <vector>

int main() {
    std::string input = "3 + 5 * 2";
    Lexer lexer(input);
    std::vector<Token> tokens = lexer.scanTokens();

    Parser parser(tokens);
    std::unique_ptr<Expr> expr = parser.parse();

    if (expr) {
        expr->print();
        std::cout << "\n✅ Parser test passed!\n";
    } else {
        std::cerr << "❌ Parser returned null.\n";
        return 1;
    }

    return 0;
}
