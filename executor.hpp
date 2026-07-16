#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP
#include <vector>
#include <string>
#include "tokens.hpp"
#include "db.hpp"
#include "condition.hpp"

// The Executor is like a smart robot!
// It takes a list of word snippets (Tokens) and does the actual work inside the database cabinet.
class Executor
{
    DB &db;                     // Reference to the database playroom cabinet.
    std::vector<Token> &tokens; // The list of chopped query word snippets we got from the Lexer.
    int idx = 0;                // Which word snippet we are currently looking at.

    // Print an error message when the query has a typo or mistake!
    void error(const std::string &s) {
        std::cout << s;
    }
    // Helper to read and construct a filter condition (like "age > 18") from tokens.
    Condition parseCondition(Table *t);

public:
    Executor(std::vector<Token> &tk, DB &db) : tokens(tk), db(db) {};
    void execute();         // Route the query to the correct action (SELECT, INSERT, etc.).
    void executeCreate();   // Create a brand new table box.
    void executeInsert();   // Insert a row card into a table.
    void executeUpdate();   // Change values on row cards in a table.
    void executeDrop();     // Throw away a table box completely.
    void executeDelete();   // Throw away row cards from a table.
    void executeSelect();   // Search and print row cards from a table.
    void executeRollback(); // Roll back a table box to an older version.
    void executeHistory();  // Print out the history log of a table box.
    void executeHelp();     // Show the user a menu of all instructions we support.
};

#endif