#ifndef VERSION_HPP
#define VERSION_HPP
#include <string>
#include <vector>

// Different types of table changes we track
enum class OperationType
{
    insert, // Row inserted
    del,    // Row deleted
    update  // Row updated
};

// Represents a historical state change of a row
struct Version
{
    OperationType type;           // Type of operations performed
    int id;                       // Row identifier that changed
    std::string key;              // The column name that was updated (for updates only)
    std::string val;              // The old value (for updates only)
    std::vector<std::string> row; // The complete row values (for inserts and deletes)
};

#endif