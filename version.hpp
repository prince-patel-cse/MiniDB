#ifndef VERSION_HPP
#define VERSION_HPP
#include <unordered_map>
enum class OperationType
{
    insert,
    del,
    update
};
struct Version
{
    OperationType type;
    std::unordered_map<std::string, std::string> row;
    int id;
    std::string key;
    std::string val;
};
#endif