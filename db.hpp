#ifndef DB_HPP
#define DB_HPP
#include "table.hpp"
#include "pager.hpp"
#include <vector>
#include <iostream>
#include "condition.hpp"

// The DB class is like a big playroom database cabinet!
// It holds many toy boxes (tables) inside.
class DB
{
public:
    int count = 0;                          // Total number of table boxes we have ever created.
    std::vector<Table> tables;              // The list of all our table boxes.
    std::unordered_map<int, int> ids;       // Maps a table's logical ID to its index position in our tables list.
    std::unordered_map<std::string, int> s; // Maps table names to their logical IDs.
    Pager pager;                            // Our pager book helper to talk to the disk file!

    DB(); // Construct our cabinet and load the file.

    void createTable(std::string name, std::vector<Column> cols); // Create a brand new table box.
    void dropTable(int id);                                      // Throw away a table box using its ID.
    void dropTable(std::string &tName);                          // Throw away a table box using its name.
    void insertRow(std::string &tName, const std::vector<std::string> &row); // Put a row card into a table.
    void updateRow(int tableId, int rowId, std::string key, std::string val); // Change a value on a row card.
    void deleteRow(int tableId, int rowId);                                   // Throw away a row card.
    void save(const std::string &filename = "");  // Save the entire cabinet to the file book on disk!
    void load(const std::string &filename = "");  // Load the entire cabinet from the file book on disk!
    void rollback(int tableId, int n);            // Roll back time for a table box to version N.
    void selectAll(int tableId, int n);           // Show table version N rows.
    void selectAll(int tableId);                  // Show all active rows of a table using its ID.
    void selectAll(std::string tName);            // Show all active rows of a table using its name.
    void selectWhere(std::string tName, Condition c);              // Select rows matching a filter screen.
    void selectColumns(std::string tName, std::vector<std::string> cols); // Select only specific columns.
};
#endif