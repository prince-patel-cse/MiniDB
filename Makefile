# Define the compiler to use (g++)
CXX = g++
# Compiler flags (use C++17 standard, show all warnings, include current directory)
CXXFLAGS = -std=c++17 -Wall -I.

# List of C++ source files
SOURCES = main.cpp db.cpp table.cpp condition.cpp lexer.cpp executor.cpp pager.cpp index.cpp
# Convert source file names to object file names (.o files)
OBJECTS = $(SOURCES:.cpp=.o)
# The name of the final compiled program
TARGET = main.exe

# The default build target is to make the executable program
all: $(TARGET)

# Link the object files together to make the final program
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile each C++ source file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Delete compiled files to clean up the directory
clean:
	del /f /q *.o $(TARGET) 2>nul || rm -f *.o $(TARGET)
