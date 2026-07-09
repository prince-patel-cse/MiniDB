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
        Version v;
        v.id = count;
        v.type = OperationType::insert;
        versions.push_back(v);
    }
}
void Table::printRow(const std::unordered_map<std::string, std::string> &row) const
{
    for (auto &col : cols)
        std::cout << col << " : " << row.at(col) << " , ";
    std::cout << std::endl;
}
void Table::printRow(const std::unordered_map<std::string, std::string> &row, std::vector<std::string> &cols) const
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
void Table::print(std::vector<std::string> &cols) const
{
    for (auto &row : rows)
        printRow(row, cols);
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
    Version v;
    v.id = id;
    v.key = key;
    v.val = rows[ids[id]][key];
    v.type = OperationType::update;
    rows[ids[id]][key] = val;
    versions.push_back(v);
}
void Table::deleteRow(int id)
{
    if (!ids.count(id))
        std::cout << "Row doesn't exists" << std::endl;
    else
    {
        Version v;
        v.id = id;
        v.row = rows[ids[id]];
        v.type = OperationType::del;
        rows.erase(rows.begin() + ids[id]);
        for (auto &p : ids)
            if (p.second > ids[id])
                p.second--;
        ids.erase(id);
        versions.push_back(v);
    }
}
void Table::findRowById(int id)
{
    if (!ids.count(id))
        std::cout << "Row doesn't exists" << std::endl;
    else
        printRow(rows[ids[id]]);
}
void Table::execute(Table &t, Version &v)
{
    if (v.type == OperationType::insert)
    {
        int idx = t.ids[v.id];
        t.rows.erase(t.rows.begin() + idx);
        t.ids.erase(v.id);
    }
    else if (v.type == OperationType::del)
    {
        t.rows.push_back(v.row);
        t.ids[v.id] = t.rows.size() - 1;
    }
    else
    {
        int idx = t.ids[v.id];
        t.rows[idx][v.key] = v.val;
    }
}
void Table::getVersion(int n)
{
    if (n < 0 || n >= versions.size())
    {
        std::cout << "Invalid version\n";
        return;
    }
    Table t = *this;
    for (int i = versions.size() - 1; i > n; i--)
        execute(t, versions[i]);
    t.print();
}
void Table::rollback(int n)
{
    if (n < 0 || n >= versions.size())
    {
        std::cout << "Invalid version\n";
        return;
    }
    Table t = *this;
    for (int i = versions.size() - 1; i > n; i--)
        execute(t, versions[i]);
    *this = t;
    while (versions.size() > n + 1)
        versions.pop_back();
}
void Table::serialize(std::ostream &out) const
{
    out << count << '\n'
        << name << '\n';
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

    for (const auto &v : versions)
    {
        out << static_cast<int>(v.type) << '\n';

        out << v.id << '\n';

        out << v.key << '\n';
        out << v.val << '\n';

        out << v.row.size() << '\n';

        for (const auto &cell : v.row)
        {
            out << cell.first << '\n';
            out << cell.second << '\n';
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
    in.ignore();

    for (size_t i = 0; i < versionCount; i++)
    {
        Version v;

        int type;
        in >> type;
        in.ignore();

        v.type = static_cast<OperationType>(type);

        in >> v.id;
        in.ignore();

        getline(in, v.key);
        getline(in, v.val);

        size_t cellCount;
        in >> cellCount;
        in.ignore();

        for (size_t j = 0; j < cellCount; j++)
        {
            std::string key, value;

            getline(in, key);
            getline(in, value);

            v.row[key] = value;
        }

        versions.push_back(v);
    }
}
