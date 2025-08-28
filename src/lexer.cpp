#include "../include/lexer.h"
#include <iostream>
#include <cctype>

// Token constructor
Token::Token(TokenType t, std::string lex, int ln)
    : type(t), lexeme(std::move(lex)), line(ln) {}

// Lexer constructor (initialize variables and keywords)
Lexer::Lexer(std::string src)
    : source(std::move(src)), start(0), current(0), line(1) {
    keywords = ({
        {"let", TokenType::LET},
        {"var", TokenType::VAR},
        {"func", TokenType::FUNC},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"summon", TokenType::SUMMON},
        {"print", TokenType::PRINT},
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT},
        {"assemble", TokenType::ASSEMBLE}
    }) {};
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line);
    return tokens;
}

bool Lexer::isAtEnd() const {
    return current >= (int)source.length();
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': addToken(TokenType::MINUS); break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                multiLineComment();
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            break;
        case '"':
            string();
            break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                std::cerr << "[Line " << line << "] Unexpected character: " << c << "\n";
                addToken(TokenType::ERROR);
            }
            break;
    }
}

char Lexer::advance() {
    return source[current++];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= (int)source.length()) return '\0';
    return source[current + 1];
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, line);
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "[Line " << line << "] Unterminated string.\n";
        addToken(TokenType::ERROR);
        return;
    }

    advance(); // Consume closing "

    std::string value = source.substr(start + 1, current - start - 2);
    tokens.emplace_back(TokenType::STRING, value, line);
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext())) {
        advance();

        while (isdigit(peek())) advance();
    }

    std::string value = source.substr(start, current - start);
    tokens.emplace_back(TokenType::NUMBER, value, line);
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = source.substr(start, current - start);
    auto keyword = keywords.find(text);
    if (keyword != keywords.end()) {
        addToken(keyword->second);
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

void Lexer::multiLineComment() {
    while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }
    if (isAtEnd()) {
        std::cerr << "[Line " << line << "] Unterminated multi-line comment.\n";
        return;
    }
    advance(); // *
    advance(); // /
}
