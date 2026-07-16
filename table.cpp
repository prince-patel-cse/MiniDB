#include "table.hpp"
#include "dataTypeValidation.hpp"
#include <cstring>
#include <iostream>

Table::Table(const std::string &name, const std::vector<Column> &cols) {
  this->cols = cols;
  this->name = name;
  // Set up our columns layout maps so we know where columns are and what types
  // they hold.
  for (size_t i = 0; i < cols.size(); ++i) {
    s[cols[i].name] = cols[i].type;
    colIndex[cols[i].name] = i;
  }
  // Initialize only the internal row ID index guide (id_idx) by default!
  indexes.emplace_back("id_idx", name, "id", cols.size(), DataTypes::INT);
  // Add a blank history item for the beginning.
  versions.push_back({});
}

bool Table::validate(const std::vector<std::string> &row) {
  // If the input row doesn't have the same number of columns as our table
  // schema, it's bad!
  if (cols.size() != row.size()) {
    std::cout << "incorrect number of cols" << std::endl;
    return false;
  }
  // Check if each item in the row matches the data type of its column.
  for (size_t i = 0; i < cols.size(); ++i) {
    if (!validType(row[i], cols[i].type)) {
      std::cout << "Invalid type for column: " << cols[i].name << std::endl;
      return false;
    }
  }
  return true; // Everything is clean and valid!
}

void Table::insertRow(const std::vector<std::string> &row) {
  // Only insert if the input row items have correct datatypes!
  if (validate(row)) {
    rows.push_back(row);
    std::cout << "Row added successfully" << std::endl
              << "ID : " << count + 1 << std::endl;
    ids[++count] =
        rows.size() - 1; // Map this row's logical ID to its memory slot.

    // Add this new row's values to all active search index guides!
    for (auto &idx : indexes) {
      if (idx.columnName == "id") {
        idx.insert(std::to_string(count), {0, 0, count});
      } else {
        size_t cIdx = colIndex[idx.columnName];
        idx.insert(row[cIdx], {0, 0, count});
      }
    }

    // Record this addition in our version history logbook!
    Version v;
    v.id = count;
    v.row = row;
    v.type = OperationType::insert;
    versions.push_back(v);
    std::cout << "Table version: " << versions.size() - 1 << std::endl;
  }
}

void Table::printRow(const std::vector<std::string> &row) const {
  // Print all columns of a row formatted nicely: "colName : value , colName2 :
  // value2"
  for (size_t i = 0; i < cols.size(); ++i) {
    std::cout << cols[i].name << " : " << row[i]
              << (i + 1 < cols.size() ? " , " : "");
  }
  std::cout << std::endl;
}

void Table::printRow(const std::vector<std::string> &row,
                     const std::vector<std::string> &colsToPrint) const {
  // Print only the columns the user asked for!
  for (size_t i = 0; i < colsToPrint.size(); ++i) {
    std::string colName = colsToPrint[i];
    if (colIndex.count(colName)) {
      size_t idx = colIndex.at(colName);
      std::cout << colName << " : " << row[idx]
                << (i + 1 < colsToPrint.size() ? " , " : "");
    }
  }
  std::cout << std::endl;
}

void Table::print(
    const std::vector<std::vector<std::string>> &rowsToPrint) const {
  for (auto &row : rowsToPrint)
    printRow(row);
}

void Table::print() const {
  for (auto &row : rows)
    printRow(row);
}

void Table::print(const std::vector<std::string> &colsToPrint) const {
  for (auto &row : rows)
    printRow(row, colsToPrint);
}

void Table::updateRow(int id, const std::string &key, const std::string &val) {
  // Check if the row actually exists!
  if (!ids.count(id)) {
    std::cout << "Row doesn't exists" << std::endl;
    return;
  }
  // Check if the column name exists!
  if (!colIndex.count(key)) {
    std::cout << "key doesn't exists" << std::endl;
    return;
  }
  size_t idx = colIndex.at(key);
  // Check if the new value fits this column's datatype box!
  if (!validType(val, cols[idx].type)) {
    std::cout << "Invalid type" << std::endl;
    return;
  }

  int pIdx = ids.at(id);
  std::string oldVal =
      rows[pIdx][idx]; // Remember the old value before we overwrite it.

  // Record this change in our version history logbook!
  Version v;
  v.id = id;
  v.key = key;
  v.val = oldVal;
  v.type = OperationType::update;

  rows[pIdx][idx] = val; // Write the new value!

  // Update our search index guides: find if there is an index for this column, and update it!
  for (auto &index : indexes) {
    if (index.columnName == key) {
      index.remove(oldVal, id);
      index.insert(val, {0, 0, id});
    }
  }

  versions.push_back(v);
  std::cout << "Table version: " << versions.size() - 1 << std::endl;
}

void Table::deleteRow(int id) {
  // Check if the row exists!
  if (!ids.count(id))
    std::cout << "Row doesn't exists" << std::endl;
  else {
    int physicalIdx = ids.at(id);
    const auto &rowValues = rows[physicalIdx];

    // Record this deletion in our version history logbook!
    Version v;
    v.id = id;
    v.row = rowValues;
    v.type = OperationType::del;

    // Remove this row's values from all our active search index guides!
    for (auto &idx : indexes) {
      if (idx.columnName == "id") {
        idx.remove(std::to_string(id), id);
      } else {
        size_t cIdx = colIndex[idx.columnName];
        idx.remove(rowValues[cIdx], id);
      }
    }

    // Erase it from memory and shift mapping offsets!
    rows.erase(rows.begin() + physicalIdx);
    for (auto &p : ids)
      if (p.second > physicalIdx)
        p.second--;
    ids.erase(id);

    versions.push_back(v);
    std::cout << "Table version: " << versions.size() - 1 << std::endl;
  }
}

void Table::findRowById(int id) {
  if (!ids.count(id))
    std::cout << "Row doesn't exists" << std::endl;
  else
    printRow(rows[ids.at(id)]);
}

void Table::execute(Table &t, Version &v) {
  // This is an undo operation to go back in time!
  if (v.type == OperationType::insert) {
    // Undo insert: throw away the inserted row!
    if (t.ids.count(v.id)) {
      int idx = t.ids[v.id];
      t.rows.erase(t.rows.begin() + idx);
      for (auto &p : t.ids)
        if (p.second > idx)
          p.second--;
      t.ids.erase(v.id);
    }
  } else if (v.type == OperationType::del) {
    // Undo delete: put the deleted row back!
    t.rows.push_back(v.row);
    t.ids[v.id] = t.rows.size() - 1;
  } else // update
  {
    // Undo update: write the old value back!
    if (t.ids.count(v.id) && t.colIndex.count(v.key)) {
      int idx = t.ids[v.id];
      size_t colIdx = t.colIndex[v.key];
      t.rows[idx][colIdx] = v.val;
    }
  }
}

void Table::getVersion(int n) {
  if (n < 0 || n >= static_cast<int>(versions.size())) {
    std::cout << "Invalid version" << std::endl;
    return;
  }
  // Make a temporary copy and undo changes backwards until we reach step N!
  Table t = *this;
  for (int i = static_cast<int>(versions.size()) - 1; i > n; i--)
    execute(t, versions[i]);
  t.print();
}

void Table::rollback(int n) {
  if (n < 0 || n >= static_cast<int>(versions.size())) {
    std::cout << "Invalid version" << std::endl;
    return;
  }
  // Undo changes backwards until we reach step N and overwrite our active
  // state!
  Table t = *this;
  for (int i = static_cast<int>(versions.size()) - 1; i > n; i--)
    execute(t, versions[i]);
  *this = t;
  // Wipe out the history entries that happened after step N.
  while (static_cast<int>(versions.size()) > n + 1)
    versions.pop_back();

  // Rebuild all our active search indexes from scratch so they are clean and match the
  // rolled-back state!
  for (auto &idx : indexes) {
    idx.memIndex.clear();
  }
  for (const auto &pair : ids) {
    int logicalId = pair.first;
    int physicalIdx = pair.second;
    const auto &row = rows[physicalIdx];
    for (auto &idx : indexes) {
      if (idx.columnName == "id") {
        idx.insert(std::to_string(logicalId), {0, 0, logicalId});
      } else {
        size_t cIdx = colIndex[idx.columnName];
        idx.insert(row[cIdx], {0, 0, logicalId});
      }
    }
  }

  std::cout << "Table version: " << versions.size() - 1 << std::endl;
}

// Chained Version Pages Serialization Helpers
static void serializeVersions(Pager &pager,
                              const std::vector<Version> &versions,
                              uint32_t &versionHeadPageId) {
  if (versions.empty()) {
    versionHeadPageId = 0;
    return;
  }

  // Ask the pager book for a blank page to start writing our version logbook!
  versionHeadPageId = pager.getNewPageId();
  uint32_t currPageId = versionHeadPageId;
  char pageBuf[Pager::PAGE_SIZE];
  std::memset(pageBuf, 0, Pager::PAGE_SIZE);

  uint16_t v_count = 0;
  uint32_t next_page_id = 0;
  size_t offset = 7;

  for (const auto &v : versions) {
    std::vector<char> vBuf;
    uint8_t type = static_cast<uint8_t>(v.type);
    vBuf.push_back(type);
    appendBytes(vBuf, v.id);

    uint16_t keyLen = static_cast<uint16_t>(v.key.size());
    appendBytes(vBuf, keyLen);
    vBuf.insert(vBuf.end(), v.key.begin(), v.key.end());

    uint16_t valLen = static_cast<uint16_t>(v.val.size());
    appendBytes(vBuf, valLen);
    vBuf.insert(vBuf.end(), v.val.begin(), v.val.end());

    uint16_t rowSize = static_cast<uint16_t>(v.row.size());
    appendBytes(vBuf, rowSize);
    for (const auto &cell : v.row) {
      uint16_t cellLen = static_cast<uint16_t>(cell.size());
      appendBytes(vBuf, cellLen);
      vBuf.insert(vBuf.end(), cell.begin(), cell.end());
    }

    // If the current version log entry does not fit on the current page, write
    // this page to disk and link a brand new page!
    if (offset + vBuf.size() > Pager::PAGE_SIZE) {
      pageBuf[0] = 3; // Version Page type marker
      std::memcpy(pageBuf + 5, &v_count, 2);
      next_page_id = pager.getNewPageId();
      std::memcpy(pageBuf + 1, &next_page_id, 4);

      pager.writePage(currPageId, pageBuf);

      currPageId = next_page_id;
      std::memset(pageBuf, 0, Pager::PAGE_SIZE);
      v_count = 0;
      offset = 7;
    }

    if (!vBuf.empty()) {
      std::memcpy(pageBuf + offset, vBuf.data(), vBuf.size());
      offset += vBuf.size();
      v_count++;
    }
  }

  // Write the last version page to disk!
  pageBuf[0] = 3;
  std::memcpy(pageBuf + 5, &v_count, 2);
  next_page_id = 0;
  std::memcpy(pageBuf + 1, &next_page_id, 4);
  pager.writePage(currPageId, pageBuf);
}

static void deserializeVersions(Pager &pager, uint32_t versionHeadPageId,
                                std::vector<Version> &versions) {
  versions.clear();
  if (versionHeadPageId == 0)
    return;

  uint32_t currPageId = versionHeadPageId;
  char pageBuf[Pager::PAGE_SIZE];

  while (currPageId != 0) {
    pager.readPage(currPageId, pageBuf);
    // Make sure it is actually a version page (Type must be 3)!
    if (pageBuf[0] != 3)
      break;

    uint16_t v_count = 0;
    uint32_t next_page_id = 0;
    std::memcpy(&v_count, pageBuf + 5, 2);
    std::memcpy(&next_page_id, pageBuf + 1, 4);

    const char *ptr = pageBuf + 7;
    for (uint16_t i = 0; i < v_count; ++i) {
      Version v;
      uint8_t type = *ptr;
      ptr++;
      v.type = static_cast<OperationType>(type);

      v.id = readBytes<int32_t>(ptr);

      uint16_t keyLen = readBytes<uint16_t>(ptr);
      v.key = std::string(ptr, keyLen);
      ptr += keyLen;

      uint16_t valLen = readBytes<uint16_t>(ptr);
      v.val = std::string(ptr, valLen);
      ptr += valLen;

      uint16_t rowSize = readBytes<uint16_t>(ptr);
      v.row.resize(rowSize);
      for (uint16_t j = 0; j < rowSize; ++j) {
        uint16_t cellLen = readBytes<uint16_t>(ptr);
        v.row[j] = std::string(ptr, cellLen);
        ptr += cellLen;
      }
      versions.push_back(v);
    }

    currPageId = next_page_id;
  }
}

void Table::serializeToPages(Pager &pager) {
  if (rows.empty()) {
    headPageId = 0;
  } else {
    // Ask the pager book for a blank page to start writing our row cards!
    headPageId = pager.getNewPageId();
    uint32_t currPageId = headPageId;
    char pageBuf[Pager::PAGE_SIZE];
    std::memset(pageBuf, 0, Pager::PAGE_SIZE);

    uint16_t num_rows = 0;
    uint32_t next_page_id = 0;
    size_t offset = 11;

    std::unordered_map<int, RowLocation> finalLocations;

    // Iterate through all our active rows in memory
    for (const auto &pair : ids) {
      int logicalId = pair.first;
      int physicalIdx = pair.second;
      const auto &row = rows[physicalIdx];

      std::vector<char> rowBuf;
      rowBuf.push_back(1); // Write '1' indicating the row is active!
      appendBytes(rowBuf, logicalId);
      // Write each column value into binary format based on datatype
      for (size_t colIdx = 0; colIdx < cols.size(); ++colIdx) {
        std::string val = row[colIdx];
        if (cols[colIdx].type == DataTypes::INT) {
          int32_t v = 0;
          try {
            v = std::stoi(val);
          } catch (...) {
          }
          appendBytes(rowBuf, v);
        } else if (cols[colIdx].type == DataTypes::DOUBLE) {
          double v = 0.0;
          try {
            v = std::stod(val);
          } catch (...) {
          }
          appendBytes(rowBuf, v);
        } else if (cols[colIdx].type == DataTypes::BOOL) {
          bool v = (val == "true" || val == "1");
          appendBytes(rowBuf, v);
        } else {
          uint16_t len = static_cast<uint16_t>(val.size());
          appendBytes(rowBuf, len);
          rowBuf.insert(rowBuf.end(), val.begin(), val.end());
        }
      }

      // If the row doesn't fit on this page, write it to disk and link a new
      // page!
      if (offset + rowBuf.size() > Pager::PAGE_SIZE) {
        pageBuf[0] = 1; // Data Page type marker
        std::memcpy(pageBuf + 1, &headPageId, 4);
        std::memcpy(pageBuf + 5, &num_rows, 2);
        next_page_id = pager.getNewPageId();
        std::memcpy(pageBuf + 7, &next_page_id, 4);

        pager.writePage(currPageId, pageBuf);

        currPageId = next_page_id;
        std::memset(pageBuf, 0, Pager::PAGE_SIZE);
        num_rows = 0;
        offset = 11;
      }

      // Record this row's final page ID and offset so we can update the index
      // guides!
      finalLocations[logicalId] = {currPageId, static_cast<uint32_t>(offset),
                                   logicalId};

      std::memcpy(pageBuf + offset, rowBuf.data(), rowBuf.size());
      offset += rowBuf.size();
      num_rows++;
    }

    // Write the last data page to disk!
    pageBuf[0] = 1;
    std::memcpy(pageBuf + 1, &headPageId, 4);
    std::memcpy(pageBuf + 5, &num_rows, 2);
    next_page_id = 0;
    std::memcpy(pageBuf + 7, &next_page_id, 4);
    pager.writePage(currPageId, pageBuf);

    // Update all our search index guides with the correct page IDs and offsets
    // we just wrote!
    for (auto &pair : finalLocations) {
      int logicalId = pair.first;
      RowLocation loc = pair.second;
      int pIdx = ids[logicalId];
      const auto &row = rows[pIdx];

      for (auto &idx : indexes) {
        if (idx.columnName == "id") {
          std::string idStr = std::to_string(logicalId);
          if (idx.memIndex.count(idStr)) {
            auto &locs = idx.memIndex[idStr];
            for (auto &l : locs) {
              if (l.logicalId == logicalId) {
                l.pageId = loc.pageId;
                l.offset = loc.offset;
              }
            }
          }
        } else {
          size_t cIdx = colIndex[idx.columnName];
          if (idx.memIndex.count(row[cIdx])) {
            auto &locs = idx.memIndex[row[cIdx]];
            for (auto &l : locs) {
              if (l.logicalId == logicalId) {
                l.pageId = loc.pageId;
                l.offset = loc.offset;
              }
            }
          }
        }
      }
    }
  }

  // Save all our search index guides to pages on disk!
  for (auto &idx : indexes) {
    idx.serialize(pager);
  }

  // Save our version logs to pages on disk!
  serializeVersions(pager, versions, versionHeadPageId);
}

void Table::deserializeFromPages(Pager &pager, uint32_t headPage,
                                 uint32_t indexRootPage) {
  // Clear out our in-memory lists before reading from the file book!
  rows.clear();
  versions.clear();
  ids.clear();
  colIndex.clear();
  s.clear();

  for (size_t i = 0; i < cols.size(); ++i) {
    s[cols[i].name] = cols[i].type;
    colIndex[cols[i].name] = i;
  }

  headPageId = headPage;
  count = 0;

  if (headPageId != 0) {
    uint32_t currPageId = headPageId;
    char pageBuf[Pager::PAGE_SIZE];

    while (currPageId != 0) {
      pager.readPage(currPageId, pageBuf);
      // Make sure this is a data page (Type must be 1)!
      if (pageBuf[0] != 1)
        break;

      uint16_t num_rows = 0;
      uint32_t next_page_id = 0;
      std::memcpy(&num_rows, pageBuf + 5, 2);
      std::memcpy(&next_page_id, pageBuf + 7, 4);

      const char *ptr = pageBuf + 11;
      for (uint16_t i = 0; i < num_rows; ++i) {
        bool active = (*ptr == 1);
        ptr++;
        int32_t logicalId = readBytes<int32_t>(ptr);

        std::vector<std::string> row;
        for (size_t colIdx = 0; colIdx < cols.size(); ++colIdx) {
          if (cols[colIdx].type == DataTypes::INT) {
            int32_t v = readBytes<int32_t>(ptr);
            row.push_back(std::to_string(v));
          } else if (cols[colIdx].type == DataTypes::DOUBLE) {
            double v = readBytes<double>(ptr);
            row.push_back(std::to_string(v));
          } else if (cols[colIdx].type == DataTypes::BOOL) {
            bool v = readBytes<bool>(ptr);
            row.push_back(v ? "true" : "false");
          } else {
            uint16_t len = readBytes<uint16_t>(ptr);
            std::string v(ptr, len);
            ptr += len;
            row.push_back(v);
          }
        }

        if (active) {
          rows.push_back(row);
          ids[logicalId] =
              rows.size() - 1; // Map the loaded row ID to its memory slot.
          if (logicalId > count) {
            count = logicalId;
          }
        }
      }

      currPageId = next_page_id;
    }
  }

  // Load our version history entries back!
  deserializeVersions(pager, versionHeadPageId, versions);
}
