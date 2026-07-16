#include "lexer.hpp"
#include <unordered_map>
#include <cctype>

Lexer::Lexer(const std::string &input)
{
    this->input = input;
    pos = 0; // Start our pointing finger at the very beginning of the sentence (index 0).
}

char Lexer::currentChar() const
{
    // If our pointing finger is past the end of the sentence, return a special null letter.
    if (pos >= input.size())
        return '\0';

    return input[pos]; // Otherwise, return the letter under our finger.
}

char Lexer::peek() const
{
    // Look at the very next letter without moving our finger pointer.
    if (pos + 1 >= input.size())
        return '\0';

    return input[pos + 1];
}

void Lexer::advance()
{
    pos++; // Shift our pointing finger one step to the right!
}

void Lexer::skipWhitespace()
{
    // If the letter is a space, tab, or newline, keep skipping it until we hit a real letter!
    while (isspace(currentChar()))
        advance();
}

Token Lexer::identifier()
{
    std::string word;
    // Keep eating letters, numbers, and underscores to form a full word.
    while (isalnum(currentChar()) || currentChar() == '_')
    {
        word += toupper(currentChar()); // Make all letters uppercase so they match keywords easily!
        advance();
    }

    // This is our vocabulary list of database words we know!
    std::unordered_map<std::string, TokenType> keywords = {
        {"CREATE", TokenType::CREATE},
        {"TABLE", TokenType::TABLE},
        {"INSERT", TokenType::INSERT},
        {"INTO", TokenType::INTO},
        {"VALUES", TokenType::VALUES},
        {"UPDATE", TokenType::UPDATE},
        {"SET", TokenType::SET},
        {"DELETE", TokenType::DELETE},
        {"FROM", TokenType::FROM},
        {"WHERE", TokenType::WHERE},
        {"SELECT", TokenType::SELECT},
        {"VERSION", TokenType::VERSION},
        {"ROLLBACK", TokenType::ROLLBACK},
        {"DROP", TokenType::DROP},
        {"INDEX", TokenType::INDEX},
        {"ON", TokenType::ON},
        {"HISTORY", TokenType::HISTORY},
        {"HELP", TokenType::HELP},

        {"INT", TokenType::INT},
        {"DOUBLE", TokenType::DOUBLE},
        {"STRING", TokenType::STRING},
        {"BOOL", TokenType::BOOL}};

    // If the word matches a keyword on our list, return it with its matching tag.
    auto it = keywords.find(word);
    if (it != keywords.end())
        return Token(it->second, word);

    // Otherwise, it is just a regular name tag chosen by the user (like a table name "users").
    return Token(TokenType::IDENTIFIER, word);
}

Token Lexer::number()
{
    std::string num;
    // Keep eating digits and decimal points to form a number.
    while (isdigit(currentChar()) || currentChar() == '.')
    {
        num += currentChar();
        advance();
    }

    return Token(TokenType::NUMBER, num);
}

Token Lexer::stringLiteral()
{
    advance(); // Skip past the opening double quotation mark.
    std::string str;

    // Eat letters inside the quotes until we hit the closing quote!
    while (currentChar() != '"' && currentChar() != '\0')
    {
        str += currentChar();
        advance();
    }

    advance(); // Skip past the closing double quotation mark.

    return Token(TokenType::STRING_LITERAL, str);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    // Keep reading until we reach the end of the sentence!
    while (currentChar() != '\0')
    {
        skipWhitespace();
        char ch = currentChar();
        if (ch == '\0')
            break;

        // If it starts with a letter, chop out a word or keyword!
        if (isalpha(ch) || ch == '_')
        {
            tokens.push_back(identifier());
            continue;
        }

        // If it starts with a digit, chop out a number!
        if (isdigit(ch))
        {
            tokens.push_back(number());
            continue;
        }

        // If it starts with a double quote, chop out a string literal value!
        if (ch == '"')
        {
            tokens.push_back(stringLiteral());
            continue;
        }

        // Check if the symbol is one of our special punctuation marks!
        switch (ch)
        {
        case '(':
            tokens.emplace_back(TokenType::LPAREN, "(");
            break;

        case ')':
            tokens.emplace_back(TokenType::RPAREN, ")");
            break;

        case ',':
            tokens.emplace_back(TokenType::COMMA, ",");
            break;

        case ';':
            tokens.emplace_back(TokenType::SEMICOLON, ";");
            break;

        case '*':
            tokens.emplace_back(TokenType::STAR, "*");
            break;

        case '=':
            tokens.emplace_back(TokenType::EQUAL, "=");
            break;

        case '<':
            tokens.emplace_back(TokenType::LT, "=");
            break;

        case '>':
            tokens.emplace_back(TokenType::GT, "=");
            break;

        default:
            // If it's something weird we don't understand, mark it as invalid!
            tokens.emplace_back(TokenType::INVALID, std::string(1, ch));
        }

        advance();
    }

    // Add a final token to say we have reached the end of the query paper.
    tokens.emplace_back(TokenType::END_OF_FILE, "");

    return tokens;
}