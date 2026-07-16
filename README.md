# MiniDB

MiniDB is a lightweight database engine written in C++ that provides time-travel functionality through the HISTORY and ROLLBACK queries.

---

## How to Compile and Run

### Step 1: Compile the Code (using CMake)
MiniDB uses CMake as its build system to handle dependencies and compile source files.

Run this command once to configure the build workspace:
```powershell
cmake -G "MinGW Makefiles" -B build
```

Then, run this command to build/compile the project:
```powershell
cmake --build build
```
This generates the executable file (`main.exe`) inside the `build/` directory.

### Step 2: Run the Database CLI
To launch the database shell:
```powershell
./build/main.exe
```
This opens the database command line interface with the `db> ` prompt. Type `exit` to save current modifications and terminate the database session.

---

## Supported Queries

MiniDB supports 10 shell commands:

1. **CREATE TABLE** (Initializes a new table schema):
   ```sql
   CREATE TABLE users (STRING name, INT id, DOUBLE salary);
   ```
2. **CREATE INDEX** (Explicitly creates a search index for a table column):
   ```sql
   CREATE INDEX idx_name ON users (name);
   ```
3. **INSERT INTO** (Inserts a new row record into a table):
   ```sql
   INSERT INTO users VALUES ("John", 1, 55000.0);
   ```
4. **SELECT FROM** (Retrieves rows from a table):
   - Select all columns:
     ```sql
     SELECT * FROM users;
     ```
   - Select specific columns:
     ```sql
     SELECT name, salary FROM users;
     ```
   - Select filtered by a conditional clause (utilizes fast index searches if an index exists):
     ```sql
     SELECT * FROM users WHERE id = 1;
     ```
5. **UPDATE SET** (Modifies existing row values):
   ```sql
   UPDATE users SET salary = 60000.0 WHERE id = 1;
   ```
6. **DELETE FROM** (Deletes rows matching a condition):
   ```sql
   DELETE FROM users WHERE id = 1;
   ```
7. **DROP TABLE** (Removes a table and its schemas completely):
   ```sql
   DROP TABLE users;
   ```
8. **ROLLBACK** (Restores table state to a specific historical version):
   ```sql
   ROLLBACK users 1;
   ```
9. **HISTORY** (Lists log entries of previous operations):
   ```sql
   HISTORY users;
   ```
10. **HELP** (Displays query guide details):
    ```sql
    HELP;
    ```

## System Architecture

MiniDB is structured similarly to professional database engines, dividing its workload into a query frontend, an indexing layer, and a page-based storage backend.

### Data Flow
1. **CLI (`main.cpp`)**: Receives the user query.
2. **Lexer (`lexer.cpp`)**: Converts the raw query string into tokens.
3. **Executor (`executor.cpp`)**: Parses syntax and executes database commands.
4. **DB (`db.cpp`)**: Orchestrates database metadata and maps logical tables.
5. **Table (`table.cpp`)**: Manages row data, schema validation, rollbacks, and indexes.
6. **Index (`index.cpp`)**: Fast map lookup pointing to binary page offsets.
7. **Pager (`pager.cpp`)**: Directly manages fixed-size 4KB binary page reads and writes on disk.

### 1. Query Processing Frontend
*   **Tokenization**: The input SQL query is processed by the **Lexer**, which converts raw text strings into discrete, categorized keywords and literals (`TokenType`).
*   **Execution routing**: The **Executor** parses the token stream to ensure correct SQL syntax. If valid, it immediately maps the query to the corresponding table operation.

### 2. Binary Storage Engine (The Pager)
*   **Page-Based Storage**: The **Pager** communicates directly with `database.db`, reading and writing fixed-size **4KB (4096-byte) blocks** called pages. The file offset is calculated as `pageId * 4096`.
*   **Page Types**:
    *   `Page 0`: Reserved for the catalog metadata, including table names, schemas, logical row ID sequences, and head page IDs.
    *   `Type 1 (Data)`: Contains active table rows serialized to binary.
    *   `Type 2 (Index)`: Contains serialized search-index maps.
    *   `Type 3 (Version)`: Contains table state log history for rollback.
*   **Page Chaining**: Since a page is 4KB, tables with many rows dynamically link multiple pages using a `next_page_id` pointer in each page header, forming a linked list of pages on disk.

### 3. Indexing Layer
*   **Address Mapping**: The **Index** maps column values (e.g. employee name `"John"`) to a `RowLocation` containing its physical disk address `{pageId, offset, logicalId}`.
*   **Tree Search**: It uses a balanced binary search tree (`std::map` / Red-Black Tree) in memory for $O(\log N)$ fast search. This allows `SELECT ... WHERE` queries to fetch records directly from the database file, bypassing slow linear table scans.

### 4. History and Rollbacks
*   Every data modification (`INSERT`, `UPDATE`, `DELETE`) is appended to a log of versions on the version pages.
*   The `ROLLBACK` command reads these logs backwards, reversing changes step-by-step to restore the table to a previous state.

---

## Component Directory

*   `main.cpp`: Entry point. Runs the interactive REPL CLI loop and manages setup/teardown.
*   `lexer.hpp` / `lexer.cpp`: Scans the SQL input and segments it into query tokens.
*   `executor.hpp` / `executor.cpp`: Validates query grammar and triggers corresponding database commands.
*   `db.hpp` / `db.cpp`: Database coordinator. Manages table creation, catalog serialization/deserialization on Page 0, and table mappings.
*   `table.hpp` / `table.cpp`: Manages logical rows, schema columns, indexes, and versioning. Performs table serialization/deserialization.
*   `pager.hpp` / `pager.cpp`: Performs low-level binary read/write operations of 4KB pages to the disk file.
*   `index.hpp` / `index.cpp`: Implements search-index maps and index serialization.
*   `condition.hpp` / `condition.cpp`: Evaluates conditional logic statements (`=`, `<`, `>`) on records.
*   `tokens.hpp`: Defines token classifications used by the lexer.
*   `column.hpp`: Defines table datatypes (`INT`, `STRING`, `DOUBLE`, `BOOL`) and columns.
*   `version.hpp`: Defines operation structures for the transaction logs.
*   `dataTypeValidation.hpp`: Provides datatype validation helpers.

