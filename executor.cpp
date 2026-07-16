#include "executor.hpp"
#include <iostream>
#include <stdexcept>
#include "column.hpp"

Condition Executor::parseCondition(Table *t)
{
    Condition cond;
    // We expect to see a column name first!
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        throw std::runtime_error("Expected column name in WHERE clause");
    }
    std::string colName = tokens[idx++].val;
    // Check if the column name actually exists in our table!
    if (!t->colIndex.count(colName))
    {
        throw std::runtime_error("Column " + colName + " does not exist in table " + t->name);
    }
    cond.colIdx = t->colIndex[colName];
    cond.type = t->cols[cond.colIdx].type;

    // Check which comparison sign we are using (=, <, or >)
    TokenType opType = tokens[idx].type;
    if (opType == TokenType::EQUAL)
    {
        cond.op = '=';
    }
    else if (opType == TokenType::LT)
    {
        cond.op = '<';
    }
    else if (opType == TokenType::GT)
    {
        cond.op = '>';
    }
    else
    {
        throw std::runtime_error("Expected comparison operator (=, <, >) in WHERE clause");
    }
    idx++;

    // Check if the target comparison value is a number, name, or text!
    if (tokens[idx].type != TokenType::NUMBER && tokens[idx].type != TokenType::STRING_LITERAL && tokens[idx].type != TokenType::IDENTIFIER)
    {
        throw std::runtime_error("Expected value in WHERE clause");
    }
    cond.val = tokens[idx++].val;
    return cond;
}

void Executor::execute()
{
    if (tokens.empty() || tokens[idx].type == TokenType::END_OF_FILE)
        return;
    TokenType type = tokens[idx].type;
    ++idx;
    // Check the first command word and run the matching function!
    if (type == TokenType::INSERT)
    {
        executeInsert();
    }
    else if (type == TokenType::DELETE)
    {
        executeDelete();
    }
    else if (type == TokenType::ROLLBACK)
    {
        executeRollback();
    }
    else if (type == TokenType::CREATE)
    {
        executeCreate();
    }
    else if (type == TokenType::SELECT)
    {
        executeSelect();
    }
    else if (type == TokenType::DROP)
    {
        executeDrop();
    }
    else if (type == TokenType::UPDATE)
    {
        executeUpdate();
    }
    else if (type == TokenType::HISTORY)
    {
        executeHistory();
    }
    else if (type == TokenType::HELP)
    {
        executeHelp();
    }
    else
        std::cout << "Invalid Query" << std::endl;
}

void Executor::executeCreate()
{
    // Expect: CREATE TABLE <name> (<type1> <col1>, ...)
    if (tokens[idx++].type != TokenType::TABLE)
    {
        error("Missing TABLE keyword\n");
        return;
    }
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;
    if (tokens[idx++].type != TokenType::LPAREN)
    {
        error("Missing (\n");
        return;
    }
    std::vector<Column> cols;
    // Loop to read all columns inside the parentheses!
    while (tokens[idx].type != TokenType::END_OF_FILE && tokens[idx].type != TokenType::RPAREN)
    {
        TokenType type = tokens[idx].type;
        Column c;
        switch (type)
        {
        case TokenType::BOOL:
            c.type = DataTypes::BOOL;
            break;
        case TokenType::DOUBLE:
            c.type = DataTypes::DOUBLE;
            break;
        case TokenType::INT:
            c.type = DataTypes::INT;
            break;
        case TokenType::STRING:
            c.type = DataTypes::STRING;
            break;
        default:
            error("Invalid column type\n");
            return;
        }
        idx++;

        if (tokens[idx].type != TokenType::IDENTIFIER)
        {
            error("Expected column name\n");
            return;
        }
        c.name = tokens[idx++].val;
        cols.push_back(c);

        if (tokens[idx].type == TokenType::COMMA)
        {
            idx++;
        }
        else if (tokens[idx].type != TokenType::RPAREN)
        {
            error("Missing , or )\n");
            return;
        }
    }

    if (tokens[idx++].type != TokenType::RPAREN)
    {
        error("Missing )\n");
        return;
    }
    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }
    db.createTable(tName, cols); // Tell the database cabinet to create the table box!
}

void Executor::executeInsert()
{
    // Expect: INSERT INTO <name> VALUES (<val1>, <val2>, ...)
    if (tokens[idx++].type != TokenType::INTO)
    {
        error("Missing INTO\n");
        return;
    }
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    if (tokens[idx++].type != TokenType::VALUES)
    {
        error("Missing VALUES\n");
        return;
    }
    if (tokens[idx++].type != TokenType::LPAREN)
    {
        error("Missing (\n");
        return;
    }

    // Look up the table box by its name.
    Table *t = nullptr;
    for (Table &table : db.tables)
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

    std::vector<std::string> rowValues;
    // Loop to read all values to insert!
    while (tokens[idx].type != TokenType::END_OF_FILE && tokens[idx].type != TokenType::RPAREN)
    {
        if (tokens[idx].type != TokenType::NUMBER && tokens[idx].type != TokenType::STRING_LITERAL && tokens[idx].type != TokenType::IDENTIFIER)
        {
            error("Invalid parameter in VALUES\n");
            return;
        }
        rowValues.push_back(tokens[idx++].val);

        if (tokens[idx].type == TokenType::COMMA)
        {
            idx++;
        }
        else if (tokens[idx].type != TokenType::RPAREN)
        {
            error("Missing , or )\n");
            return;
        }
    }

    if (tokens[idx++].type != TokenType::RPAREN)
    {
        error("Missing )\n");
        return;
    }
    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    // Make sure we got the exact same number of values as columns!
    if (rowValues.size() != t->cols.size())
    {
        std::cout << "Incorrect number of columns: expected " << t->cols.size() << ", got " << rowValues.size() << "\n";
        return;
    }

    t->insertRow(rowValues);
}

void Executor::executeDrop()
{
    // Expect: DROP TABLE <name>;
    if (tokens[idx++].type != TokenType::TABLE)
    {
        error("Missing TABLE\n");
        return;
    }
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;
    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }
    db.dropTable(tName);
}

void Executor::executeDelete()
{
    // Expect: DELETE FROM <name> [WHERE ...]
    if (tokens[idx++].type != TokenType::FROM)
    {
        error("Missing FROM\n");
        return;
    }
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    Table *t = nullptr;
    for (Table &table : db.tables)
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

    Condition cond;
    bool hasWhere = false;
    // Check if there is a filter screen (WHERE)!
    if (tokens[idx].type == TokenType::WHERE)
    {
        idx++;
        try
        {
            cond = parseCondition(t);
            hasWhere = true;
        }
        catch (const std::exception &e)
        {
            error(std::string(e.what()) + "\n");
            return;
        }
    }

    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    // Collect all rows that match the filter condition to throw away!
    std::vector<int> idsToDelete;
    for (const auto &p : t->ids)
    {
        int logicalId = p.first;
        int physicalIdx = p.second;
        if (!hasWhere || cond.isTrue(t->rows[physicalIdx]))
        {
            idsToDelete.push_back(logicalId);
        }
    }

    for (int id : idsToDelete)
    {
        t->deleteRow(id);
    }
    std::cout << "Deleted " << idsToDelete.size() << " rows" << std::endl;
}

void Executor::executeUpdate()
{
    // Expect: UPDATE <name> SET <col> = <val> [WHERE ...]
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    Table *t = nullptr;
    for (Table &table : db.tables)
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

    if (tokens[idx++].type != TokenType::SET)
    {
        error("Expected SET keyword\n");
        return;
    }

    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected column name in SET\n");
        return;
    }
    std::string setCol = tokens[idx++].val;
    if (!t->colIndex.count(setCol))
    {
        std::cout << "Column " << setCol << " does not exist in table\n";
        return;
    }

    if (tokens[idx++].type != TokenType::EQUAL)
    {
        error("Expected =\n");
        return;
    }

    if (tokens[idx].type != TokenType::NUMBER && tokens[idx].type != TokenType::STRING_LITERAL && tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected value in SET\n");
        return;
    }
    std::string setVal = tokens[idx++].val;

    Condition cond;
    bool hasWhere = false;
    if (tokens[idx].type == TokenType::WHERE)
    {
        idx++;
        try
        {
            cond = parseCondition(t);
            hasWhere = true;
        }
        catch (const std::exception &e)
        {
            error(std::string(e.what()) + "\n");
            return;
        }
    }

    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    // Collect all rows that match the filter condition to update!
    int updateCount = 0;
    std::vector<int> idsToUpdate;
    for (const auto &p : t->ids)
    {
        int logicalId = p.first;
        int physicalIdx = p.second;
        if (!hasWhere || cond.isTrue(t->rows[physicalIdx]))
        {
            idsToUpdate.push_back(logicalId);
        }
    }

    for (int id : idsToUpdate)
    {
        t->updateRow(id, setCol, setVal);
        updateCount++;
    }
    std::cout << "Updated " << updateCount << " rows" << std::endl;
}

void Executor::executeRollback()
{
    // Expect: ROLLBACK <name> <version_number>;
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    Table *t = nullptr;
    for (Table &table : db.tables)
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

    if (tokens[idx].type != TokenType::NUMBER)
    {
        error("Expected version number\n");
        return;
    }
    int versionNum = std::stoi(tokens[idx++].val);

    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    t->rollback(versionNum);
}

void Executor::executeHistory()
{
    // Expect: HISTORY <name> [limit];
    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    Table *t = nullptr;
    for (Table &table : db.tables)
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

    int limit = -1;
    // Check if the user specified a print count limit number.
    if (tokens[idx].type == TokenType::NUMBER)
    {
        limit = std::stoi(tokens[idx++].val);
    }

    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }
    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    std::cout << "History of table " << t->name << ":" << std::endl;
    int startIdx = 0;
    if (limit > 0 && t->versions.size() > static_cast<size_t>(limit))
    {
        startIdx = t->versions.size() - limit;
    }

    // Print all history log entries from our start offset!
    for (size_t i = startIdx; i < t->versions.size(); ++i)
    {
        const auto &v = t->versions[i];
        std::cout << "Version " << i << ": ";
        if (i == 0)
        {
            std::cout << "Initial empty state" << std::endl;
            continue;
        }

        switch (v.type)
        {
        case OperationType::insert:
        {
            std::cout << "INSERT row ID " << v.id << " values (";
            for (size_t j = 0; j < v.row.size(); ++j)
            {
                std::cout << v.row[j] << (j + 1 < v.row.size() ? ", " : "");
            }
            std::cout << ")" << std::endl;
            break;
        }
        case OperationType::del:
        {
            std::cout << "DELETE row ID " << v.id << " values (";
            for (size_t j = 0; j < v.row.size(); ++j)
            {
                std::cout << v.row[j] << (j + 1 < v.row.size() ? ", " : "");
            }
            std::cout << ")" << std::endl;
            break;
        }
        case OperationType::update:
        {
            std::cout << "UPDATE row ID " << v.id << ": set " << v.key << ", old value was: " << v.val << std::endl;
            break;
        }
        }
    }
}

void Executor::executeSelect()
{
    // Expect: SELECT * FROM <name> [WHERE ...] or SELECT col1, col2 FROM ...
    std::vector<std::string> selectCols;
    bool selectAll = false;
    if (tokens[idx].type == TokenType::STAR)
    {
        selectAll = true;
        idx++;
    }
    else
    {
        // Loop to parse all projected columns we want to show!
        while (true)
        {
            if (tokens[idx].type != TokenType::IDENTIFIER)
            {
                error("Expected column name or * in SELECT\n");
                return;
            }
            selectCols.push_back(tokens[idx++].val);
            if (tokens[idx].type == TokenType::COMMA)
            {
                idx++;
            }
            else
            {
                break;
            }
        }
    }

    if (tokens[idx++].type != TokenType::FROM)
    {
        error("Expected FROM keyword\n");
        return;
    }

    if (tokens[idx].type != TokenType::IDENTIFIER)
    {
        error("Expected table name after FROM\n");
        return;
    }
    std::string tName = tokens[idx++].val;

    Table *t = nullptr;
    for (Table &table : db.tables)
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

    Condition cond;
    bool hasWhere = false;
    // Check if there is a filter condition (WHERE)!
    if (tokens[idx].type == TokenType::WHERE)
    {
        idx++;
        try
        {
            cond = parseCondition(t);
            hasWhere = true;
        }
        catch (const std::exception &e)
        {
            error(std::string(e.what()) + "\n");
            return;
        }
    }

    if (tokens[idx].type == TokenType::SEMICOLON)
    {
        idx++;
    }

    if (tokens[idx].type != TokenType::END_OF_FILE)
    {
        error("Query not ended successfully\n");
        return;
    }

    // Execute select!
    if (selectAll)
    {
        if (hasWhere)
        {
            db.selectWhere(tName, cond);
        }
        else
        {
            t->print();
        }
    }
    else
    {
        // Verify columns exist
        for (const auto &col : selectCols)
        {
            if (!t->colIndex.count(col))
            {
                std::cout << "Column " << col << " does not exist in table " << t->name << std::endl;
                return;
            }
        }
        if (hasWhere)
        {
            // Accelerated index lookup:
            // Find if there is an index guide for the column in our condition
            Index *colIdx = nullptr;
            for (auto &idx : t->indexes)
            {
                if (idx.columnName == t->cols[cond.colIdx].name)
                {
                    colIdx = &idx;
                    break;
                }
            }
            // If guide exists and comparison is '=', use it to read only matching pages!
            if (colIdx && cond.op == '=')
            {
                auto locs = colIdx->lookup(cond.val);
                for (const auto &loc : locs)
                {
                    if (t->ids.count(loc.logicalId))
                    {
                        int pIdx = t->ids.at(loc.logicalId);
                        t->printRow(t->rows[pIdx], selectCols);
                    }
                }
            }
            else
            {
                // Otherwise read all rows one-by-one to check (table scan)!
                for (const auto &row : t->rows)
                {
                    if (cond.isTrue(row))
                    {
                        t->printRow(row, selectCols);
                    }
                }
            }
        }
        else
        {
            t->print(selectCols);
        }
    }
}

void Executor::executeHelp()
{
    // Print all supported syntaxes in easy language!
    std::cout << "\nSupported Queries in MiniDB:\n"
              << "=========================================================================\n"
              << "1. CREATE TABLE   : CREATE TABLE <tableName> (<colType> <colName>, ...);\n"
              << "                    Types: INT, STRING, DOUBLE, BOOL\n"
              << "                    Example: CREATE TABLE users (STRING name, INT id);\n\n"
              << "2. INSERT INTO    : INSERT INTO <tableName> VALUES (<val1>, <val2>, ...);\n"
              << "                    Example: INSERT INTO users VALUES (\"Alice\", 1);\n\n"
              << "3. SELECT FROM    : SELECT * FROM <tableName> [WHERE <colName> <op> <val>];\n"
              << "                    SELECT <col1>, <col2> FROM <tableName> [WHERE ...];\n"
              << "                    Operators: =, <, >\n"
              << "                    Example: SELECT * FROM users WHERE id = 1;\n\n"
              << "4. UPDATE SET     : UPDATE <tableName> SET <colName> = <val> [WHERE ...];\n"
              << "                    Example: UPDATE users SET name = \"Bob\" WHERE id = 1;\n\n"
              << "5. DELETE FROM    : DELETE FROM <tableName> [WHERE <colName> <op> <val>];\n"
              << "                    Example: DELETE FROM users WHERE id = 1;\n\n"
              << "6. DROP TABLE     : DROP TABLE <tableName>;\n"
              << "                    Example: DROP TABLE users;\n\n"
              << "7. ROLLBACK       : ROLLBACK <tableName> <version_number>;\n"
              << "                    Example: ROLLBACK users 1;\n\n"
              << "8. HISTORY        : HISTORY <tableName> [limit];\n"
              << "                    Example: HISTORY users 5;\n\n"
              << "9. HELP           : HELP;\n"
              << "                    Displays this help information.\n"
              << "=========================================================================\n\n";
}