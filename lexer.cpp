#include "lexer.hpp"
#include <unordered_map>
#include <cctype>

Lexer::Lexer(const std::string &input)
{
    this->input = input;
    pos = 0;
}

char Lexer::currentChar() const
{
    if (pos >= input.size())
        return '\0';

    return input[pos];
}

char Lexer::peek() const
{
    if (pos + 1 >= input.size())
        return '\0';

    return input[pos + 1];
}

void Lexer::advance()
{
    pos++;
}

void Lexer::skipWhitespace()
{
    while (isspace(currentChar()))
        advance();
}

Token Lexer::identifier()
{
    std::string word;
    while (isalnum(currentChar()) || currentChar() == '_')
    {
        word += toupper(currentChar());
        advance();
    }

    std::unordered_map<std::string, TokenType> keywords = {
        {"CREATE", TokenType::CREATE},
        {"TABLE", TokenType::TABLE},
        {"INSERT", TokenType::INSERT},
        {"INTO", TokenType::INTO},
        {"VALUES", TokenType::VALUES},
        {"UPDATE", TokenType::UPDATE},
        {"SET", TokenType::SET},
        {"DELETE", TokenType::DELETE_},
        {"FROM", TokenType::FROM},
        {"WHERE", TokenType::WHERE},
        {"SELECT", TokenType::SELECT},
        {"VERSION", TokenType::VERSION},
        {"ROLLBACK", TokenType::ROLLBACK},

        {"INT", TokenType::INT},
        {"DOUBLE", TokenType::DOUBLE},
        {"STRING", TokenType::STRING},
        {"BOOL", TokenType::BOOL}};

    auto it = keywords.find(word);
    if (it != keywords.end())
        return Token(it->second, word);

    return Token(TokenType::IDENTIFIER, word);
}
Token Lexer::number()
{

    std::string num;

    while (isdigit(currentChar()) || currentChar() == '.')
    {
        num += currentChar();
        advance();
    }

    return Token(TokenType::NUMBER, num);
}
Token Lexer::stringLiteral()
{

    advance();
    std::string str;

    while (currentChar() != '"' && currentChar() != '\0')
    {
        str += currentChar();
        advance();
    }

    advance();

    return Token(TokenType::STRING_LITERAL, str);
}
std::vector<Token> Lexer::tokenize()
{

    std::vector<Token> tokens;
    while (currentChar() != '\0')
    {
        skipWhitespace();
        char ch = currentChar();
        if (ch == '\0')
            break;

        if (isalpha(ch) || ch == '_')
        {
            tokens.push_back(identifier());
            continue;
        }

        if (isdigit(ch))
        {
            tokens.push_back(number());
            continue;
        }

        if (ch == '"')
        {
            tokens.push_back(stringLiteral());
            continue;
        }

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
            tokens.emplace_back(TokenType::INVALID,
                                std::string(1, ch));
        }

        advance();
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "");

    return tokens;
}