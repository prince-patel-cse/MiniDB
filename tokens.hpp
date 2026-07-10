#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType
{
    // Keywords
    CREATE,
    TABLE,
    INSERT,
    INTO,
    VALUES,
    UPDATE,
    SET,
    DELETE_,
    FROM,
    WHERE,
    SELECT,
    VERSION,

    INT,
    STRING,
    BOOL,
    DOUBLE,

    // Symbols
    LPAREN,
    RPAREN,
    COMMA,
    SEMICOLON,
    STAR,
    EQUAL,
    LT,
    GT,
    ROLLBACK,

    // Literals
    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,

    END_OF_FILE,
    INVALID
};

struct Token
{
    TokenType type;
    std::string val;

    Token(TokenType t, const std::string &l)
    {
        type = t;
        val = l;
    }
};

#endif