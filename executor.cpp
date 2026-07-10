#include "executor.hpp"
#include <iostream>
#include "column.hpp"
void Executor::execute()
{
    TokenType type = tokens[idx].type;
    ++idx;
    if (type == TokenType::INSERT)
    {
        executeInsert();
    }
    else if (type == TokenType::DELETE)
    {
        executeDelete();
    }
    else if (type == TokenType::ROLLBACK)
    {
        executeRollback();
    }
    else if (type == TokenType::CREATE)
    {
        executeCreate();
    }
    else if (type == TokenType::SELECT)
    {
        executeSelect();
    }
    else if (type == TokenType::DROP)
    {
        executeDrop();
    }
    else if (type == TokenType::UPDATE)
    {
        executeUpdate();
    }
    else
        std::cout << "Invalid Query\n";
}
void Executor::executeCreate()
{
    if (tokens[idx++].type != TokenType::INTO)
    {
        error("Missing INTO\n");
        return;
    }
    std::string tName = tokens[idx++].val;
    if (tokens[idx++].type != TokenType::VALUES)
    {
        error("Missing VALUES\n");
        return;
    }
    if (tokens[idx++].type != TokenType::LPAREN)
    {
        error("Missing (");
        return;
    }
    std::vector<Column> cols;
    while (tokens[idx].type != TokenType::END_OF_FILE && tokens[idx].type != TokenType::RPAREN)
    {
        TokenType type = tokens[idx].type;
        Column c;
        switch (type)
        {
        case TokenType::BOOL:
        {
            c.type = DataTypes::BOOL;
            break;
        }
        case TokenType::DOUBLE:
        {
            c.type = DataTypes::DOUBLE;
            break;
        }
        case TokenType::INT:
        {
            c.type = DataTypes::INT;
            break;
        }
        case TokenType::STRING:
        {
            c.type = DataTypes::STRING;
            break;
        }
        default:
            error("Invalid query\n");
        }
        idx++;
        if (tokens[idx].type == TokenType::END_OF_FILE || tokens[idx].type == TokenType::RPAREN)
        {
            error("Invalid query\n");
            return;
        }
        c.name = tokens[idx].val;
        cols.push_back(c);
    }
    if (tokens[idx++].type != TokenType::RPAREN)
    {
        error("Missing )");
        return;
    }
    if (tokens[idx++].type != TokenType::SEMICOLON)
    {
        error("Missing ;\n");
        return;
    }
    db.createTable(tName, cols);
}
void error(std::string s)
{
    std::cout << s;
}