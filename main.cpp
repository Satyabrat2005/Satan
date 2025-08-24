#include <iostream>
#include "lexer.h"
#include "parser.h"

int main() {
    std::string code = R"(
        let x = 10;
        var name = "SatanLang";
        summon "Hello, World!";
        // This is a comment
        /*
         Multi-line comment
        */
        if (x >= 10) {
            summon "x is large";
        }
    )";

    Lexer lexer(code);
    auto tokens = lexer.scanTokens();

    for (const auto& token : tokens) {
        std::cout << "Line " << token.line << ": " << token.lexeme << " (Type: " << static_cast<int>(token.type) << ")\n";
    }

    std::cout << "\n--- Parsing ---\n";
    auto statements = parser.parse();

    if (!statements.empty()) {
        for (const auto& stmt : statements) {
            if (stmt) {
                stmt->print();
                std::cout << std::endl;
            }
        }
    } else {
        std::cerr << "Parsing failed.\n";
    }


    return 0;
}
