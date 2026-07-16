#include "db.hpp"
#include "lexer.hpp"
#include "executor.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

int main()
{
    DB db;
    // Ask our playroom cabinet to load all tables and rows from the "database.db" file!
    db.load();

    std::string query;
    std::cout << "\nWelcome to MiniDB\nexit : to exit menu , help for help\n\n";
    // Keep running in a loop to ask the user for instructions!
    while (true)
    {
        std::cout << "db> ";
        std::getline(std::cin, query); // Get the command typed by the user.

        // If the user says "exit", save all our toy boxes and close the program!
        if (query == "exit")
        {
            db.save();
            break;
        }

        // If the command is empty, do nothing and wait for another command.
        if (query.empty())
            continue;

        try
        {
            // 1. Give the command to the Lexer to chop it into clean word snippets!
            Lexer lexer(query);
            std::vector<Token> tokens = lexer.tokenize();
            if (tokens.empty() || tokens[0].type == TokenType::END_OF_FILE)
            {
                continue;
            }
            // 2. Give the word snippets to our Executor robot to do the actual work!
            Executor executor(tokens, db);
            executor.execute();
            // 3. Save changes to our disk file immediately so we never lose our toys!
            db.save();
        }
        catch (const std::exception &e)
        {
            // If the query has mistakes, tell the user!
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}