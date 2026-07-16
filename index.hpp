#ifndef INDEX_HPP
#define INDEX_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>
#include "column.hpp"
#include "pager.hpp"

// Stores the page position and offset location of a row in the database file
struct RowLocation
{
    uint32_t pageId;  // Page index where the row is located
    uint32_t offset;  // Byte offset of the row within the page
    int logicalId;    // Logical unique identifier of the row
};

// Writes raw bytes of an object into a vector buffer
template <typename T>
inline void appendBytes(std::vector<char> &buf, const T &val)
{
    const char *ptr = reinterpret_cast<const char *>(&val);
    buf.insert(buf.end(), ptr, ptr + sizeof(T));
}

// Reads raw bytes of an object from a pointer and advances the pointer
template <typename T>
inline T readBytes(const char *&ptr)
{
    T val;
    std::memcpy(&val, ptr, sizeof(T));
    ptr += sizeof(T);
    return val;
}

// Manages a balanced lookup index for table column values
class Index
{
public:
    std::string name;        // Name of the index file/structure
    std::string tableName;   // Name of the indexed table
    std::string columnName;  // Name of the column this index is for
    size_t colIdx;           // Index of the column within the schema
    DataTypes colType;       // DataType of the indexed values
    uint32_t rootPageId;     // The root binary page ID where the index is saved

    // In-memory key-to-locations map for fast lookup
    std::map<std::string, std::vector<RowLocation>> memIndex;

    Index(const std::string &name, const std::string &tName, const std::string &cName, size_t colIdx, DataTypes colType);

    void insert(const std::string &key, RowLocation loc); // Inserts a key location pair
    void remove(const std::string &key, int logicalId);  // Removes a key location pair
    std::vector<RowLocation> lookup(const std::string &key) const; // Looks up matching locations

    void serialize(Pager &pager);                        // Saves the index to binary pages
    void deserialize(Pager &pager, uint32_t startPageId); // Loads the index from binary pages
};

#endif
