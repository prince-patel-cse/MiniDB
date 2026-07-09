#ifndef DB_HPP
#define DB_HPP
#include "table.hpp"
#include <vector>
#include <iostream>
class DB
{
public:
    int count = 0;
    std::vector<Table> tables;
    std::unordered_map<int, int> ids;
    std::unordered_set<std::string> s;
    void createTable(std::string name, std::vector<std::string> cols);
    void dropTable(int id);
    void insertRow(int id, const std::unordered_map<std::string, std::string> row);
    void updateRow(int tableId, int rowId, std::string key, std::string val);
    void deleteRow(int tableId, int rowId);
    void save(const std::string &filename) const;
    void load(const std::string &filename);
    void rollback(int tableId, int n);
    void selectAll(int tableId, int n);
    void selectAll(int tableId);
    void selectAll(std::string tName);
    void selectWhere(std::string tName, std::string key, std::string val);
    void selectColumns(std::string tName, std::vector<std::string> cols);
    void selectColumnsWhere(std::string tName, std::vector<std::string> cols, std::string key, std::string val);
};
#endif