#include "db.hpp"
#include <iostream>

int main()
{
    DB db;

    db.createTable("Users", {"name", "age", "city"});

    db.insertRow(1, {{"name", "Alice"},
                     {"age", "21"},
                     {"city", "London"}});

    db.insertRow(1, {{"name", "Bob"},
                     {"age", "22"},
                     {"city", "Delhi"}});

    db.updateRow(1, 1, "city", "Mumbai");

    db.deleteRow(1, 2);

    std::cout << "\n========== VERSION 0 ==========\n";
    db.selectAll(1, 0);

    std::cout << "\n========== VERSION 1 ==========\n";
    db.selectAll(1, 1);

    std::cout << "\n========== VERSION 2 ==========\n";
    db.selectAll(1, 2);

    std::cout << "\n========== VERSION 3 ==========\n";
    db.selectAll(1, 3);

    std::cout << "\n========== VERSION 4 ==========\n";
    db.selectAll(1, 4);

    db.rollback(1, 2);
    db.selectAll(1);

    std::cout << "\nSaving database...\n";
    db.save("database.txt");
    return 0;
}