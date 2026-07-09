#ifndef COLUMN_HPP
#define COLUMN_HPP
#include <string>
enum class DataTypes
{
    INT,
    STRING,
    BOOL,
    DOUBLE
};

struct Column
{
    DataTypes type;
    std::string name;
};

#endif