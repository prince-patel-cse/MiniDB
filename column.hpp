#ifndef COLUMN_HPP
#define COLUMN_HPP
#include <string>

// Supported data types in the database
enum class DataTypes
{
    INT,     // Integer numbers
    STRING,  // Text strings
    DOUBLE,  // Decimal numbers
    BOOL     // Boolean values (true or false)
};

// Represents a column in a table schema
struct Column
{
    DataTypes type;   // The data type of the column
    std::string name; // The name of the column
};

#endif