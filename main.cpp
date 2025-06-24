#include <iostream>
#include "lexer.h"

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

    return 0;
}
