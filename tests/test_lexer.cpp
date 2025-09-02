#include "../include/lexer.h"
#include <iostream>
#include <cassert>


std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LET: return "LET";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::SEMICOLON: return "SEMICOLON";
        default: return "OTHER";
    }
}

int main() {
    std::string source = "let x = 42;";
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    assert(tokens.size() >= 5); // LET, IDENTIFIER, EQUAL, NUMBER, SEMICOLON

    std::cout << "Token stream:\n";
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << " : " << token.lexeme << " (line " << token.line << ")\n";
    }

    assert(tokens[0].type == TokenType::LET);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[2].type == TokenType::EQUAL);
    assert(tokens[3].type == TokenType::NUMBER);
    assert(tokens[3].lexeme == "42");
    assert(tokens[4].type == TokenType::SEMICOLON);

    std::cout << " Lexer test passed successfully.\n";
    return 0;
}
