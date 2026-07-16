#include "db.hpp"
#include <fstream>
#include <cstring>
#include "dataTypeValidation.hpp"
#include "condition.hpp"

DB::DB() : pager("database.db") {}

void DB::createTable(std::string name, std::vector<Column> cols)
{
    // If the table box already exists, tell the user and do nothing!
    if (s.count(name))
    {
        std::cout << "Table already exists\n";
        return;
    }
    Table t(name, cols);
    tables.push_back(t);
    ids[++count] = tables.size() - 1; // Map this new logical ID to its memory slot.
    std::cout << "Table created successfully\nID : " << count << std::endl;
    s[name] = count;
}

void DB::dropTable(int id)
{
    // Check if table ID is valid!
    if (!ids.count(id))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    int physicalIdx = ids[id];
    std::string tableName = tables[physicalIdx].name;
    tables.erase(tables.begin() + physicalIdx); // Delete from our vector!
    // Shift table index offsets for the remaining tables.
    for (auto &p : ids)
        if (p.second > physicalIdx)
            p.second--;
    ids.erase(id);
    s.erase(tableName);
    std::cout << "Table dropped successfully\n";
}

void DB::dropTable(std::string &tName)
{
    // Check if table name exists!
    if (!s.count(tName))
    {
        std::cout << "Table does not exist" << std::endl;
        return;
    }
    int id = s[tName];
    if (!ids.count(id))
    {
        std::cout << "Invalid id" << std::endl;
        return;
    }
    int physicalIdx = ids[id];
    tables.erase(tables.begin() + physicalIdx);
    for (auto &p : ids)
        if (p.second > physicalIdx)
            p.second--;
    ids.erase(id);
    s.erase(tName);
    std::cout << "Table dropped successfully\n";
}

void DB::insertRow(std::string &tName, const std::vector<std::string> &row)
{
    Table *t = nullptr;
    for (Table &table : tables)
    {
        if (table.name == tName)
        {
            t = &table;
            break;
        }
    }
    if (!t)
    {
        std::cout << "Table does not exist\n";
        return;
    }
    t->insertRow(row);
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

void DB::save(const std::string &filename)
{
    pager.clear(); // Empty the disk file first!
    uint32_t catId = pager.getNewPageId(); // Reserve Page 0 for our catalog index

    // First, serialize all table rows, versions, and indexes to get their disk page IDs!
    for (auto &table : tables)
    {
        table.serializeToPages(pager);
    }

    char catBuf[Pager::PAGE_SIZE];
    std::memset(catBuf, 0, Pager::PAGE_SIZE);

    // Write signature and general database variables to Page 0
    std::memcpy(catBuf, "MINIDB\0", 7);
    uint32_t num_tables = static_cast<uint32_t>(tables.size());
    std::memcpy(catBuf + 7, &num_tables, 4);
    std::memcpy(catBuf + 11, &count, 4);

    uint32_t ids_size = static_cast<uint32_t>(ids.size());
    std::memcpy(catBuf + 15, &ids_size, 4);

    std::vector<char> catalogData;
    // Append logical ID mappings
    for (const auto &p : ids)
    {
        appendBytes(catalogData, p.first);
        appendBytes(catalogData, p.second);
    }

    // Append table schemas, columns, types, and their disk page IDs!
    for (const auto &table : tables)
    {
        uint8_t nameLen = static_cast<uint8_t>(table.name.size());
        catalogData.push_back(nameLen);
        catalogData.insert(catalogData.end(), table.name.begin(), table.name.end());

        appendBytes(catalogData, table.headPageId);
        appendBytes(catalogData, table.versionHeadPageId);

        uint16_t colCount = static_cast<uint16_t>(table.cols.size());
        appendBytes(catalogData, colCount);
        for (const auto &col : table.cols)
        {
            uint8_t colNameLen = static_cast<uint8_t>(col.name.size());
            catalogData.push_back(colNameLen);
            catalogData.insert(catalogData.end(), col.name.begin(), col.name.end());
            uint8_t typeVal = static_cast<uint8_t>(col.type);
            catalogData.push_back(typeVal);
        }

        uint16_t idxCount = static_cast<uint16_t>(table.indexes.size());
        appendBytes(catalogData, idxCount);
        for (const auto &idx : table.indexes)
        {
            // Save index name
            uint8_t idxNameLen = static_cast<uint8_t>(idx.name.size());
            catalogData.push_back(idxNameLen);
            catalogData.insert(catalogData.end(), idx.name.begin(), idx.name.end());

            // Save column name
            uint8_t idxColNameLen = static_cast<uint8_t>(idx.columnName.size());
            catalogData.push_back(idxColNameLen);
            catalogData.insert(catalogData.end(), idx.columnName.begin(), idx.columnName.end());

            // Save root page ID
            appendBytes(catalogData, idx.rootPageId);
        }
    }

    // Make sure catalog list fits on our Page 0 buffer limit
    if (catalogData.size() > 4077)
    {
        std::cout << "Error: Catalog exceeds Page 0 size limit." << std::endl;
        return;
    }

    if (!catalogData.empty())
    {
        std::memcpy(catBuf + 19, catalogData.data(), catalogData.size());
    }

    pager.writePage(catId, catBuf); // Write Page 0 to the disk file!
}

void DB::load(const std::string &filename)
{
    tables.clear();
    ids.clear();
    s.clear();
    count = 0;

    // If the database file is blank, do nothing!
    if (pager.getTotalPages() == 0)
    {
        std::cout << "Database loaded successfully (empty database).\n";
        return;
    }

    char catBuf[Pager::PAGE_SIZE];
    pager.readPage(0, catBuf);

    // Make sure the file signature matches!
    if (std::memcmp(catBuf, "MINIDB\0", 7) != 0)
    {
        std::cout << "Error: Invalid database file signature." << std::endl;
        return;
    }

    uint32_t num_tables = 0;
    std::memcpy(&num_tables, catBuf + 7, 4);
    std::memcpy(&count, catBuf + 11, 4);

    uint32_t ids_size = 0;
    std::memcpy(&ids_size, catBuf + 15, 4);

    const char *ptr = catBuf + 19;

    // Read logical table mappings back!
    for (uint32_t i = 0; i < ids_size; i++)
    {
        if (ptr + 8 > catBuf + Pager::PAGE_SIZE)
        {
            std::cout << "Error: Corrupted database file.\n";
            tables.clear(); ids.clear(); s.clear(); count = 0;
            return;
        }
        int logicalId = readBytes<int32_t>(ptr);
        int index = readBytes<int32_t>(ptr);
        ids[logicalId] = index;
    }

    // Read all tables schemas, columns, search indexes and rows!
    for (uint32_t i = 0; i < num_tables; i++)
    {
        if (ptr + 1 > catBuf + Pager::PAGE_SIZE)
        {
            std::cout << "Error: Corrupted database file.\n";
            tables.clear(); ids.clear(); s.clear(); count = 0;
            return;
        }
        uint8_t nameLen = *ptr; ptr++;
        if (ptr + nameLen + 10 > catBuf + Pager::PAGE_SIZE)
        {
            std::cout << "Error: Corrupted database file.\n";
            tables.clear(); ids.clear(); s.clear(); count = 0;
            return;
        }
        std::string tableName(ptr, nameLen);
        ptr += nameLen;

        uint32_t headPageId = readBytes<uint32_t>(ptr);
        uint32_t versionHeadPageId = readBytes<uint32_t>(ptr);

        uint16_t colCount = readBytes<uint16_t>(ptr);
        std::vector<Column> cols(colCount);
        for (uint16_t c = 0; c < colCount; c++)
        {
            if (ptr + 1 > catBuf + Pager::PAGE_SIZE)
            {
                std::cout << "Error: Corrupted database file.\n";
                tables.clear(); ids.clear(); s.clear(); count = 0;
                return;
            }
            uint8_t colNameLen = *ptr; ptr++;
            if (ptr + colNameLen + 1 > catBuf + Pager::PAGE_SIZE)
            {
                std::cout << "Error: Corrupted database file.\n";
                tables.clear(); ids.clear(); s.clear(); count = 0;
                return;
            }
            cols[c].name = std::string(ptr, colNameLen);
            ptr += colNameLen;
            uint8_t typeVal = *ptr; ptr++;
            cols[c].type = static_cast<DataTypes>(typeVal);
        }

        Table table(tableName, cols);
        table.headPageId = headPageId;
        table.versionHeadPageId = versionHeadPageId;

        // Read and load every search index back!
        if (ptr + 2 > catBuf + Pager::PAGE_SIZE)
        {
            std::cout << "Error: Corrupted database file.\n";
            tables.clear(); ids.clear(); s.clear(); count = 0;
            return;
        }
        uint16_t idxCount = readBytes<uint16_t>(ptr);
        for (uint16_t idx = 0; idx < idxCount; idx++)
        {
            if (ptr + 1 > catBuf + Pager::PAGE_SIZE)
            {
                std::cout << "Error: Corrupted database file.\n";
                tables.clear(); ids.clear(); s.clear(); count = 0;
                return;
            }
            uint8_t idxNameLen = *ptr; ptr++;
            if (ptr + idxNameLen + 1 > catBuf + Pager::PAGE_SIZE)
            {
                std::cout << "Error: Corrupted database file.\n";
                tables.clear(); ids.clear(); s.clear(); count = 0;
                return;
            }
            std::string indexName = std::string(ptr, idxNameLen);
            ptr += idxNameLen;

            uint8_t idxColNameLen = *ptr; ptr++;
            if (ptr + idxColNameLen + 4 > catBuf + Pager::PAGE_SIZE)
            {
                std::cout << "Error: Corrupted database file.\n";
                tables.clear(); ids.clear(); s.clear(); count = 0;
                return;
            }
            std::string colName = std::string(ptr, idxColNameLen);
            ptr += idxColNameLen;
            uint32_t rootPageId = readBytes<uint32_t>(ptr);

            if (colName == "id")
            {
                for (auto &tableIdx : table.indexes)
                {
                    if (tableIdx.columnName == "id")
                    {
                        tableIdx.rootPageId = rootPageId;
                        tableIdx.deserialize(pager, rootPageId);
                        break;
                    }
                }
            }
            else
            {
                if (table.colIndex.count(colName))
                {
                    size_t colIdx = table.colIndex[colName];
                    DataTypes colType = table.cols[colIdx].type;
                    table.indexes.emplace_back(indexName, tableName, colName, colIdx, colType);
                    table.indexes.back().rootPageId = rootPageId;
                    table.indexes.back().deserialize(pager, rootPageId);
                }
            }
        }

        table.deserializeFromPages(pager, headPageId, 0); // Deserialize table row data!

        tables.push_back(table);
        s[tableName] = i;
    }

    // Rebuild active table name mappings in memory!
    for (size_t i = 0; i < tables.size(); i++)
    {
        int logicalId = -1;
        for (const auto &p : ids)
        {
            if (p.second == static_cast<int>(i))
            {
                logicalId = p.first;
                break;
            }
        }
        if (logicalId != -1)
        {
            s[tables[i].name] = logicalId;
        }
        else
        {
            s[tables[i].name] = static_cast<int>(i);
        }
    }

    std::cout << "Database loaded successfully from binary pages.\n";
}

void DB::selectAll(std::string tName)
{
    Table *t = nullptr;
    for (Table &table : tables)
    {
        if (table.name == tName)
        {
            t = &table;
            break;
        }
    }
    if (!t)
        std::cout << "Table does not exist\n";
    else
        t->print();
}

void DB::selectWhere(std::string tName, Condition c)
{
    Table *t = nullptr;
    for (Table &table : tables)
    {
        if (table.name == tName)
        {
            t = &table;
            break;
        }
    }
    if (!t)
    {
        std::cout << "Table does not exist\n";
        return;
    }

    // Index lookup acceleration helper!
    // Try to find if there is an index guide matching the column in our condition
    Index *colIdx = nullptr;
    for (auto &idx : t->indexes)
    {
        if (idx.columnName == t->cols[c.colIdx].name)
        {
            colIdx = &idx;
            break;
        }
    }

    // If an index guide is found and we are searching for '=' (equality), use the guide!
    if (colIdx && c.op == '=')
    {
        auto matchingLocations = colIdx->lookup(c.val);
        for (const auto &loc : matchingLocations)
        {
            if (t->ids.count(loc.logicalId))
            {
                int physicalIdx = t->ids.at(loc.logicalId);
                t->printRow(t->rows[physicalIdx]);
            }
        }
    }
    else
    {
        // Otherwise, read every row one-by-one to check (table scan)!
        for (const auto &row : t->rows)
        {
            if (c.isTrue(row))
            {
                t->printRow(row);
            }
        }
    }
}

void DB::selectColumns(std::string tName, std::vector<std::string> cols)
{
    Table *t = nullptr;
    for (Table &table : tables)
    {
        if (table.name == tName)
        {
            t = &table;
            break;
        }
    }
    if (!t)
    {
        std::cout << "Table does not exist\n";
        return;
    }
    t->print(cols);
}
