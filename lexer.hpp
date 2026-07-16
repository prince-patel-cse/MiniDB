#ifndef LEXER_H
#define LEXER_H

#include "tokens.hpp"
#include <vector>

// The Lexer is like a scissors and label guide!
// It takes a long query sentence like "SELECT * FROM users" and cuts it 
// into clean individual word snippets (Tokens) like "SELECT", "*", "FROM", "users".
class Lexer
{
public:
    Lexer(const std::string &input); // Make a lexer tool for a query sentence.
    std::vector<Token> tokenize();    // Chop the query sentence into a list of word snippets!

private:
    std::string input; // The query sentence we are looking at.
    size_t pos;        // Where our pointing finger is on the sentence letters.

    char currentChar() const; // Look at the letter under our pointing finger.
    char peek() const;        // Look at the next letter without moving our finger.
    void advance();           // Move our pointing finger one letter to the right.
    void skipWhitespace();    // Skip over spaces, tabs, and newlines.

    Token identifier();    // Chop out a regular word or keyword tag.
    Token number();        // Chop out a number value.
    Token stringLiteral(); // Chop out text sitting inside quotation marks "like this".
};

#endif