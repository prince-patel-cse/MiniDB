#ifndef PAGER_HPP
#define PAGER_HPP

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

// Manages binary reading and writing of fixed-size pages to the database file
class Pager
{
private:
    std::string filename;       // Name of the database file on disk
    mutable std::fstream file;  // File stream used for binary read/write operations
    uint32_t totalPages;        // Total number of pages loaded into memory

public:
    static const size_t PAGE_SIZE = 4096; // Fixed size of each binary page in bytes

    Pager(const std::string &dbFilename);
    ~Pager();

    void readPage(uint32_t pageId, char *buffer) const;  // Reads pageId from file into buffer
    void writePage(uint32_t pageId, const char *buffer); // Writes pageId from buffer to file
    uint32_t getNewPageId();                             // Appends a new blank page to the file
    uint32_t getTotalPages() const;                       // Returns the total number of pages in the file
    void flush();                                         // Flushes stream changes to disk
    void clear();                                         // Truncates and clears the database file
};

#endif
