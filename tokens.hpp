#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// List of all recognized token types parsed by the lexer
enum class TokenType
{
    // Query command words
    CREATE,
    TABLE,
    INSERT,
    INTO,
    VALUES,
    UPDATE,
    SET,
    DELETE,
    FROM,
    WHERE,
    SELECT,
    VERSION,
    DROP,

    // Data type keywords
    INT,
    STRING,
    BOOL,
    DOUBLE,

    // Command punctuation and operator symbols
    LPAREN,     // '('
    RPAREN,     // ')'
    COMMA,      // ','
    SEMICOLON,  // ';'
    STAR,       // '*'
    EQUAL,      // '='
    LT,         // '<'
    GT,         // '>'
    ROLLBACK,
    HISTORY,
    HELP,

    // Literal and name representations
    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,

    END_OF_FILE, // End of the input query
    INVALID      // Lexer encountered an unrecognized character
};

// Represents a parsed token
struct Token
{
    TokenType type;  // The type of the token
    std::string val; // The original text value of the token

    Token(TokenType t, const std::string &l)
    {
        type = t;
        val = l;
    }
};

#endif