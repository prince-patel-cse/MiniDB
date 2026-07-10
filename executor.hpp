#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP
#include <vector>
#include <string>
#include "tokens.hpp"
#include "db.hpp"
class Executor
{
    DB &db;
    std::vector<Token> &tokens;
    int idx = 0;

public:
    Executor(std::vector<Token> &tk, DB &db) : tokens(tk), db(db) {};
    void execute();
    void executeCreate();
    void executeInsert();
    void executeUpdate();
    void executeDrop();
    void executeDelete();
    void executeSelect();
    void executeRollback();
};

#endif