#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE, // for blocks
    COMMA, DOT, SEMICOLON,
    PLUS, MINUS, STAR, SLASH,
    BANG, EQUAL,
    LESS, GREATER,

    // One or two character tokens
    BANG_EQUAL, EQUAL_EQUAL,
    LESS_EQUAL, GREATER_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    LET, VAR, FUNC,
    IF, ELSE, WHILE, FOR, RETURN,
    PRINT, TRUE, FALSE,
    AND, OR, NOT, ASSEMBLE,

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
    explicit Lexer(std::string src);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    int start;
    int current;
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

#endif // LEXER_H
