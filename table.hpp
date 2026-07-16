#ifndef TABLE_HPP
#define TABLE_HPP
#include "version.hpp"
#include "column.hpp"
#include "pager.hpp"
#include "index.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

// A Table is like a box of toys! It holds many toy cards (rows).
// Each toy card has details on it, written under column name tags (columns).
class Table
{
public:
    int count = 0;              // Total number of rows we have ever added to this table box.
    std::string name;           // The name of our table box (like "EMPLOYEES").
    std::vector<Column> cols;   // The list of column slots in this table box.
    std::unordered_map<std::string, DataTypes> s;    // Maps column names to their datatypes.
    std::unordered_map<std::string, size_t> colIndex; // Maps column names to their slot position (0, 1, 2...).
    std::vector<std::vector<std::string>> rows;       // All the row values stored in memory.
    std::vector<Version> versions;                    // The history of what was added or changed.
    std::unordered_map<int, int> ids;                 // Maps a row's ID to its index position in our rows list.

    uint32_t headPageId = 0;        // The first page number on disk where this table's rows are saved.
    uint32_t versionHeadPageId = 0; // The first page number on disk where our history is saved.
    std::vector<Index> indexes;     // Pointers to the search index guides for each column!

    Table(const std::string &name, const std::vector<Column> &cols);
    bool validate(const std::vector<std::string> &row); // Checks if a row fits our columns types.
    void insertRow(const std::vector<std::string> &row); // Insert a new row card.
    void print(const std::vector<std::vector<std::string>> &rowsToPrint) const; // Print some rows.
    void print() const;                                                         // Print all rows.
    void print(const std::vector<std::string> &colsToPrint) const;              // Print specific columns.
    void printRow(const std::vector<std::string> &row) const;                   // Print a single row.
    void printRow(const std::vector<std::string> &row, const std::vector<std::string> &colsToPrint) const;
    void updateRow(int id, const std::string &key, const std::string &val);      // Change a value in a row.
    void deleteRow(int id);       // Remove a row card.
    void findRowById(int idx);    // Find and print a row using its logical ID.
    void getVersion(int n);       // Show the table exactly how it looked at version step N.
    void rollback(int n);         // Go back in time to version step N!

    // Page-based binary serialization and deserialization
    void serializeToPages(Pager &pager); // Save the table and its guides to binary pages.
    void deserializeFromPages(Pager &pager, uint32_t headPage, uint32_t indexRootPage); // Load from binary pages.

    void execute(Table &t, Version &v); // Undo helper to execute a change.
};

#endif