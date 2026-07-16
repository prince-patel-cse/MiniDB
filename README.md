# MiniDB

MiniDB is a lightweight database engine written in C++ that stores records using a binary page-based storage layout.

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

MiniDB supports 9 shell commands:

1. **CREATE TABLE** (Initializes a new table schema):
   ```sql
   CREATE TABLE users (STRING name, INT id, DOUBLE salary);
   ```
2. **INSERT INTO** (Inserts a new row record into a table):
   ```sql
   INSERT INTO users VALUES ("John", 1, 55000.0);
   ```
3. **SELECT FROM** (Retrieves rows from a table):
   - Select all columns:
     ```sql
     SELECT * FROM users;
     ```
   - Select specific columns:
     ```sql
     SELECT name, salary FROM users;
     ```
   - Select filtered by a conditional clause (utilizes fast index searches):
     ```sql
     SELECT * FROM users WHERE id = 1;
     ```
4. **UPDATE SET** (Modifies existing row values):
   ```sql
   UPDATE users SET salary = 60000.0 WHERE id = 1;
   ```
5. **DELETE FROM** (Deletes rows matching a condition):
   ```sql
   DELETE FROM users WHERE id = 1;
   ```
6. **DROP TABLE** (Removes a table and its schemas completely):
   ```sql
   DROP TABLE users;
   ```
7. **ROLLBACK** (Restores table state to a specific historical version):
   ```sql
   ROLLBACK users 1;
   ```
8. **HISTORY** (Lists log entries of previous operations):
   ```sql
   HISTORY users;
   ```
9. **HELP** (Displays query guide details):
   ```sql
   HELP;
   ```

---

## Sharing the Project

To share the MiniDB project with others:

### 1. Clean Up Build Files
Delete all temporary and generated files to keep the zip file light:
*   Delete the **`build/`** directory.
*   Delete **`main.exe`** and any other generated executables.
*   Delete **`database.db`** (resets the database state).

### 2. Required Source Files
Include the following files in your zip bundle:
*   **Source Files (.cpp and .hpp)**: All source and header files (`main.cpp`, `db.cpp`, `db.hpp`, `table.cpp`, `table.hpp`, `condition.cpp`, `condition.hpp`, `lexer.cpp`, `lexer.hpp`, `executor.cpp`, `executor.hpp`, `pager.cpp`, `pager.hpp`, `index.cpp`, `index.hpp`, `tokens.hpp`, `column.hpp`, `version.hpp`, `dataTypeValidation.hpp`)
*   **Build configurations**: `CMakeLists.txt` and `Makefile`
*   **Documentation**: `README.md`

### 3. Packaging
Compress these selected source files into a ZIP archive (e.g., `MiniDB.zip`) and distribute it.
