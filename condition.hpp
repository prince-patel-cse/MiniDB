#ifndef CONDITION_HPP
#define CONDITION_HPP
#include "column.hpp"
#include <string>
#include <vector>

// A Condition is like a filter screen! It helps us check if a row matches what we are searching for, 
// like checking if "age > 18" or "name = Bob".
struct Condition
{
    size_t colIdx = 0;                  // Which column slot are we checking in the row?
    std::string val;                    // What word or number are we comparing against?
    DataTypes type = DataTypes::STRING; // What type of box is it (number, decimal, text)?
    char op = '=';                      // The compare operation sign (can be '=', '<', or '>')

    // This checks if a single row passes our test! Returns true if it does, false if it fails.
    bool isTrue(const std::vector<std::string> &row) const;
};

#endif