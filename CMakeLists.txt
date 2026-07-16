# Set the minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.10)
# Name of the project
project(MiniDB CXX)

# Use C++17 standard for compiling the code
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# List of all source files to compile into the final executable program
add_executable(main
    main.cpp
    db.cpp
    table.cpp
    condition.cpp
    lexer.cpp
    executor.cpp
    pager.cpp
    index.cpp
)
