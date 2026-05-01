#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    COMMA, DOT, SEMICOLON, COLON,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    BANG, EQUAL,
    LESS, GREATER,

    // One or two character tokens
    BANG_EQUAL, EQUAL_EQUAL,
    LESS_EQUAL, GREATER_EQUAL,
    ARROW,      // =>
    PIPE_ARROW, // |>

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Core keywords
    LET, VAR, FUNC, FUN,
    IF, ELSE, WHILE, FOR, RETURN,
    PRINT, TRUE, FALSE,
    AND, OR, NOT, ASSEMBLE, SUMMON,
    BREAK, CONTINUE,

    // AI/ML/DL/NLP/DS keywords
    IMPORT,

    // Phase 1: Core language expansion
    TRY, CATCH,
    IN,
    ASSERT, TEST,

    // Special
    EOF_TOKEN,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    Token(TokenType t, std::string lex, int ln);
};

class Lexer {
public:
    static constexpr size_t MAX_SOURCE_SIZE = 10 * 1024 * 1024;

    explicit Lexer(std::string src);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    size_t start;
    size_t current;
    int line;

    std::unordered_map<std::string, TokenType> keywords;

    bool isAtEnd() const;
    void scanToken();
    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    void addToken(TokenType type);
    void string();
    void number();
    void identifier();
    void multiLineComment();
};

#endif
