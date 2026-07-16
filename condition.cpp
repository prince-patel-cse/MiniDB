#include "condition.hpp"
#include "dataTypeValidation.hpp"
#include <stdexcept>

bool Condition::isTrue(const std::vector<std::string> &row) const
{
    // If we want to check a column index that is too big, it's invalid, so return false!
    if (colIdx >= row.size())
        return false;
    std::string v = row[colIdx]; // Get the value sitting in our row's column slot.
    try
    {
        // Case 1: Check if the value is EQUAL to our search target!
        if (op == '=')
        {
            if (type == DataTypes::INT)
            {
                return std::stoi(v) == std::stoi(val);
            }
            else if (type == DataTypes::DOUBLE)
            {
                return std::stod(v) == std::stod(val);
            }
            else if (type == DataTypes::BOOL)
            {
                bool b1 = (v == "true" || v == "1");
                bool b2 = (val == "true" || val == "1");
                return b1 == b2;
            }
            else
            {
                return v == val;
            }
        }
        // Case 2: Check if the value is LESS THAN our search target!
        else if (op == '<')
        {
            if (type == DataTypes::INT)
            {
                return std::stoi(v) < std::stoi(val);
            }
            else if (type == DataTypes::DOUBLE)
            {
                return std::stod(v) < std::stod(val);
            }
            else
            {
                return v < val;
            }
        }
        // Case 3: Check if the value is GREATER THAN our search target!
        else if (op == '>')
        {
            if (type == DataTypes::INT)
            {
                return std::stoi(v) > std::stoi(val);
            }
            else if (type == DataTypes::DOUBLE)
            {
                return std::stod(v) > std::stod(val);
            }
            else
            {
                return v > val;
            }
        }
    }
    catch (...)
    {
        // If conversion fails (like trying to convert "hello" to a number), return false!
        return false;
    }
    return false;
}