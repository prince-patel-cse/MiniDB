#ifndef TABLE_HPP
#define TABLE_HPP
#include "version.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
// Only declares that the function exists
class Table
{
public:
    int count = 0;
    std::string name;
    std::vector<std::string> cols;
    std::unordered_set<std::string> s;
    std::vector<std::unordered_map<std::string, std::string>> rows;
    std::vector<Version> versions;
    std::unordered_map<int, int> ids;
    Table(const std::string &name, const std::vector<std::string> &cols);
    bool validate(const std::unordered_map<std::string, std::string> &row);
    void insertRow(const std::unordered_map<std::string, std::string> &row);
    void print(std::vector<std::unordered_map<std::string, std::string>> &rows) const;
    void print() const;
    void print(std::vector<std::string> &cols) const;
    void printRow(const std::unordered_map<std::string, std::string> &row) const;
    void printRow(const std::unordered_map<std::string, std::string> &row, std::vector<std::string> &cols) const;
    void updateRow(int id, std::string &key, std::string &val);
    void deleteRow(int id);
    void findRowById(int idx);
    void getVersion(int n);
    void rollback(int n);
    void serialize(std::ostream &out) const;
    void deserialize(std::istream &in);
    void execute(Table &t, Version &v);
};

#endif