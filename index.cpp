#include "index.hpp"
#include <cstring>
#include <iostream>

Index::Index(const std::string &name, const std::string &tName, const std::string &cName, size_t colIdx, DataTypes colType)
    : name(name), tableName(tName), columnName(cName), colIdx(colIdx), colType(colType), rootPageId(0) {}

void Index::insert(const std::string &key, RowLocation loc)
{
    // Append the location to the list of matching keys
    memIndex[key].push_back(loc);
}

void Index::remove(const std::string &key, int logicalId)
{
    if (memIndex.count(key))
    {
        auto &vec = memIndex[key];
        for (auto it = vec.begin(); it != vec.end(); ++it)
        {
            if (it->logicalId == logicalId)
            {
                vec.erase(it);
                break;
            }
        }
        // Remove the key entry if no locations are left
        if (vec.empty())
        {
            memIndex.erase(key);
        }
    }
}

std::vector<RowLocation> Index::lookup(const std::string &key) const
{
    if (memIndex.count(key))
    {
        return memIndex.at(key);
    }
    return {};
}

void Index::serialize(Pager &pager)
{
    // Allocate a page ID for the index if it has not been assigned yet
    if (rootPageId == 0)
    {
        rootPageId = pager.getNewPageId();
    }

    uint32_t currPageId = rootPageId;
    char pageBuf[Pager::PAGE_SIZE];
    std::memset(pageBuf, 0, Pager::PAGE_SIZE);

    auto it = memIndex.begin();
    while (it != memIndex.end())
    {
        uint16_t num_keys = 0;
        uint32_t next_page_id = 0;

        std::vector<char> data;
        while (it != memIndex.end())
        {
            std::vector<char> entry;
            std::string key = it->first;
            const auto &locs = it->second;

            // Serialize key values based on data types
            if (colType == DataTypes::INT)
            {
                try {
                    int32_t k = std::stoi(key);
                    appendBytes(entry, k);
                } catch (...) {
                    int32_t k = 0;
                    appendBytes(entry, k);
                }
            }
            else if (colType == DataTypes::DOUBLE)
            {
                try {
                    double k = std::stod(key);
                    appendBytes(entry, k);
                } catch (...) {
                    double k = 0.0;
                    appendBytes(entry, k);
                }
            }
            else if (colType == DataTypes::BOOL)
            {
                bool k = (key == "true" || key == "1");
                appendBytes(entry, k);
            }
            else
            {
                uint16_t len = static_cast<uint16_t>(key.size());
                appendBytes(entry, len);
                entry.insert(entry.end(), key.begin(), key.end());
            }

            // Serialize locations list size and contents
            uint16_t locCount = static_cast<uint16_t>(locs.size());
            appendBytes(entry, locCount);
            for (const auto &loc : locs)
            {
                appendBytes(entry, loc.logicalId);
                appendBytes(entry, loc.pageId);
                appendBytes(entry, loc.offset);
            }

            // If the serialized key exceeds page bounds, allocate a next page pointer
            if (data.size() + entry.size() > 4085)
            {
                next_page_id = pager.getNewPageId();
                break;
            }

            data.insert(data.end(), entry.begin(), entry.end());
            num_keys++;
            it++;
        }

        // Set index page header (Page Type 2, keys count, next page ID) and copy data
        pageBuf[0] = 2; // Type = Index Page
        std::memcpy(pageBuf + 1, &num_keys, 2);
        std::memcpy(pageBuf + 3, &next_page_id, 4);
        if (!data.empty())
        {
            std::memcpy(pageBuf + 7, data.data(), data.size());
        }

        pager.writePage(currPageId, pageBuf);

        // Move to the next page in the chain if configured
        if (next_page_id != 0)
        {
            currPageId = next_page_id;
            std::memset(pageBuf, 0, Pager::PAGE_SIZE);
        }
        else
        {
            break;
        }
    }
}

void Index::deserialize(Pager &pager, uint32_t startPageId)
{
    memIndex.clear();
    rootPageId = startPageId;
    if (rootPageId == 0) return;

    uint32_t currPageId = rootPageId;
    char pageBuf[Pager::PAGE_SIZE];

    while (currPageId != 0)
    {
        pager.readPage(currPageId, pageBuf);
        if (pageBuf[0] != 2) break; // Check index page marker

        uint16_t num_keys = 0;
        uint32_t next_page_id = 0;
        std::memcpy(&num_keys, pageBuf + 1, 2);
        std::memcpy(&next_page_id, pageBuf + 3, 4);

        const char *ptr = pageBuf + 7;
        for (uint16_t i = 0; i < num_keys; ++i)
        {
            std::string key;
            // Parse keys based on datatypes
            if (colType == DataTypes::INT)
            {
                int32_t k = readBytes<int32_t>(ptr);
                key = std::to_string(k);
            }
            else if (colType == DataTypes::DOUBLE)
            {
                double k = readBytes<double>(ptr);
                key = std::to_string(k);
            }
            else if (colType == DataTypes::BOOL)
            {
                bool k = readBytes<bool>(ptr);
                key = k ? "true" : "false";
            }
            else
            {
                uint16_t len = readBytes<uint16_t>(ptr);
                key = std::string(ptr, len);
                ptr += len;
            }

            // Parse locations list
            uint16_t locCount = readBytes<uint16_t>(ptr);
            std::vector<RowLocation> locs(locCount);
            for (uint16_t j = 0; j < locCount; ++j)
            {
                locs[j].logicalId = readBytes<int32_t>(ptr);
                locs[j].pageId = readBytes<uint32_t>(ptr);
                locs[j].offset = readBytes<uint32_t>(ptr);
            }
            memIndex[key] = locs;
        }

        currPageId = next_page_id;
    }
}
