#include "table.hpp"
#include <iostream>
Table::Table(const std::string &name, const std::vector<std::string> &cols)
{
    this->cols = cols;
    this->name = name;
    for (auto &col : cols)
        s.insert(col);
    versions.push_back({});
};
bool Table::validate(const std::unordered_map<std::string, std::string> &row)
{
    for (auto &c : row)
        if (!s.count(c.first))
        {
            std::cout << "Invalid row" << std::endl;
            return false;
        }
    if (s.size() != row.size())
    {
        std::cout << "incorrect number of cols" << std::endl;
        return false;
    }
    return true;
}
void Table::insertRow(const std::unordered_map<std::string, std::string> &row)
{
    if (validate(row))
    {
        rows.push_back(row);
        std::cout << "Row added successfully" << std::endl
                  << "ID : " << count + 1 << std::endl;
        ids[++count] = rows.size() - 1;
        versions.push_back(rows);
    }
}
void Table::printRow(const std::unordered_map<std::string, std::string> &row) const
{
    for (auto &col : cols)
        std::cout << col << " : " << row.at(col) << " , ";
    std::cout << std::endl;
}
void Table::print(std::vector<std::unordered_map<std::string, std::string>> &rows) const
{
    for (auto &row : rows)
        printRow(row);
}
void Table::print() const
{
    for (auto &row : rows)
        printRow(row);
}
void Table::updateRow(int id, std::string &key, std::string &val)
{
    if (!ids.count(id))
    {
        std::cout << "Row doesn't exists" << std::endl;
        return;
    }
    if (!s.count(key))
    {
        std::cout << "key doesn't exists" << std::endl;
        return;
    }
    rows[ids[id]][key] = val;
    versions.push_back(rows);
}
void Table::deleteRow(int id)
{
    if (!ids.count(id))
        std::cout << "Row doesn't exists" << std::endl;
    else
    {
        rows.erase(rows.begin() + ids[id]);
        for (auto &p : ids)
            if (p.second > ids[id])
                p.second--;
        ids.erase(id);
        versions.push_back(rows);
    }
}
void Table::findRowById(int id)
{
    if (!ids.count(id))
        std::cout << "Row doesn't exists" << std::endl;
    else
        printRow(rows[ids[id]]);
}
void Table::getVersion(int n)
{
    if (n < 0 || n > versions.size())
        std::cout << "Invalid version\n";
    else
        print(versions[n]);
}
void Table::rollback(int n)
{
    if (n < 0 || n > versions.size())
        std::cout << "Invalid version\n";
    else
        rows = versions[n];
}
void Table::serialize(std::ostream &out) const
{
    out << count << '\n' << name << '\n';
    out << cols.size() << '\n';
    for (const auto &col : cols)
        out << col << '\n';
        
    out << ids.size() << '\n';
    for (const auto &p : ids)
        out << p.first << ' ' << p.second << '\n';

    out << rows.size() << '\n';
    for (const auto &row : rows)
    {
        out << row.size() << '\n';
        for (const auto &cell : row)
            out << cell.first << '\n'
                << cell.second << '\n';
    }

    out << versions.size() << '\n';

    for (const auto &version : versions)
    {
        out << version.size() << '\n';

        for (const auto &row : version)
        {
            out << row.size() << '\n';

            for (const auto &cell : row)
            {
                out << cell.first << '\n';
                out << cell.second << '\n';
            }
        }
    }
}
void Table::deserialize(std::istream &in)
{
    cols.clear();
    rows.clear();
    versions.clear();
    ids.clear();
    s.clear();

    in >> count;
    in.ignore();

    getline(in, name);

    size_t columnCount;
    in >> columnCount;
    in.ignore();

    for (size_t i = 0; i < columnCount; i++)
    {
        std::string col;
        getline(in, col);

        cols.push_back(col);
        s.insert(col);
    }

    size_t idCount;
    in >> idCount;

    for (size_t i = 0; i < idCount; i++)
    {
        int a, b;
        in >> a >> b;
        ids[a] = b;
    }

    size_t rowCount;
    in >> rowCount;

    for (size_t i = 0; i < rowCount; i++)
    {
        size_t cellCount;
        in >> cellCount;
        in.ignore();

        std::unordered_map<std::string, std::string> row;

        for (size_t j = 0; j < cellCount; j++)
        {
            std::string key, value;

            getline(in, key);
            getline(in, value);

            row[key] = value;
        }

        rows.push_back(row);
    }

    size_t versionCount;
    in >> versionCount;

    for (size_t i = 0; i < versionCount; i++)
    {
        size_t versionRowCount;
        in >> versionRowCount;

        std::vector<std::unordered_map<std::string, std::string>> version;

        for (size_t j = 0; j < versionRowCount; j++)
        {
            size_t cellCount;
            in >> cellCount;
            in.ignore();

            std::unordered_map<std::string, std::string> row;

            for (size_t k = 0; k < cellCount; k++)
            {
                std::string key, value;

                getline(in, key);
                getline(in, value);

                row[key] = value;
            }

            version.push_back(row);
        }

        versions.push_back(version);
    }
}
