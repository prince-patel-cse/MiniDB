#ifndef LEXER_H
#define LEXER_H

#include "tokens.hpp";
#include <vector>

class Lexer
{
public:
    Lexer(const std::string &input);
    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos;

    char currentChar() const;
    char peek() const;
    void advance();
    void skipWhitespace();

    Token identifier();
    Token number();
    Token stringLiteral();
};

#endif