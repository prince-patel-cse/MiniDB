#include "db.hpp"
#include <fstream>
void DB::createTable(std::string name, std::vector<std::string> cols)
{
    Table t(name, cols);
    tables.push_back(t);
    ids[++count] = tables.size() - 1;
    std::cout << "Table created successfully\nID : " << count << std::endl;
}
void DB::dropTable(int id)
{
    if (!ids.count(id))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables.erase(tables.begin() + ids[id]);
    for (auto &p : ids)
        if (p.second > ids[id])
            p.second--;
    ids.erase(id);
}
void DB::insertRow(int id, const std::unordered_map<std::string, std::string> row)
{
    if (!ids.count(id))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables[ids[id]].insertRow(row);
}
void DB::updateRow(int tableId, int rowId, std::string key, std::string val)
{
    if (!ids.count(tableId))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables[ids[tableId]].updateRow(rowId, key, val);
}
void DB::deleteRow(int tableId, int rowId)
{
    if (!ids.count(tableId))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables[ids[tableId]].deleteRow(rowId);
}
void DB::selectAll(int tableId, int n)
{
    if (!ids.count(tableId))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables[ids[tableId]].getVersion(n);
}
void DB::selectAll(int tableId)
{
    if (!ids.count(tableId))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    tables[ids[tableId]].print();
}
void DB::rollback(int tableId, int n)
{
    if (!ids.count(tableId))
    {
        std::cout << "Invalid id\n";
        return;
    }
    tables[ids[tableId]].rollback(n);
}
void DB::save(const std::string &filename) const
{
    std::ofstream out(filename);

    out << count << '\n';

    out << ids.size() << '\n';

    for (const auto &p : ids)
        out << p.first << ' ' << p.second << '\n';

    out << tables.size() << '\n';

    for (const auto &table : tables)
        table.serialize(out);
}
void DB::load(const std::string &filename)
{
    std::ifstream in(filename);

    if (!in)
        return;

    tables.clear();
    ids.clear();

    in >> count;

    size_t idCount;
    in >> idCount;

    for (size_t i = 0; i < idCount; i++)
    {
        int a, b;
        in >> a >> b;
        ids[a] = b;
    }

    size_t tableCount;
    in >> tableCount;

    for (size_t i = 0; i < tableCount; i++)
    {
        Table table("", {});

        table.deserialize(in);

        tables.push_back(table);
    }
}