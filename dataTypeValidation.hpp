#include <string>
#include "column.hpp"
#ifndef DATATYPE_HPP
#define DATATYPE_HPP

// Returns true if a string is a valid integer
inline bool isInt(const std::string &s)
{
    try
    {
        size_t pos;
        std::stoi(s, &pos);
        return pos == s.size(); // True if the entire string was parsed as an integer
    }
    catch (...)
    {
        return false;
    }
}

// Returns true if a string is a valid double/decimal number
inline bool isDouble(const std::string &s)
{
    try
    {
        size_t pos;
        std::stod(s, &pos);
        return pos == s.size(); // True if the entire string was parsed as a double
    }
    catch (...)
    {
        return false;
    }
}

// Returns true if a string represents a boolean ("true" or "false")
inline bool isBool(const std::string &s)
{
    return s == "true" || s == "false";
}

// Validates whether a value string matches its expected datatype
inline bool validType(const std::string &str, const DataTypes &type)
{
    switch (type)
    {
    case DataTypes::INT:
        if (!isInt(str))
            return false;
        break;

    case DataTypes::DOUBLE:
        if (!isDouble(str))
            return false;
        break;

    case DataTypes::BOOL:
        if (!isBool(str))
            return false;
        break;

    case DataTypes::STRING:
        // Any string is a valid string representation
        break;
    }
    return true;
}
#endif