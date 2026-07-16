#include "pager.hpp"
#include <iostream>
#include <cstring>

Pager::Pager(const std::string &dbFilename) : filename(dbFilename), totalPages(0)
{
    // Try opening file in read/write mode. Create it if it does not exist.
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        std::ofstream create(filename, std::ios::binary);
        create.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (file.is_open())
    {
        // Seek to the end of the file to determine file size and calculate pages
        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        totalPages = static_cast<uint32_t>(size / PAGE_SIZE);
    }
}

Pager::~Pager()
{
    if (file.is_open())
    {
        file.close();
    }
}

void Pager::readPage(uint32_t pageId, char *buffer) const
{
    std::memset(buffer, 0, PAGE_SIZE);
    if (!file.is_open()) return;

    // Do not read if pageId is out of bounds
    if (pageId >= totalPages)
    {
        return;
    }

    // Seek to the start of the page and read the page size bytes
    file.seekg(static_cast<std::streamoff>(pageId) * PAGE_SIZE, std::ios::beg);
    file.read(buffer, PAGE_SIZE);
}

void Pager::writePage(uint32_t pageId, const char *buffer)
{
    if (!file.is_open()) return;

    // Seek to the start of the target page and write page data
    file.seekp(static_cast<std::streamoff>(pageId) * PAGE_SIZE, std::ios::beg);
    file.write(buffer, PAGE_SIZE);
    file.flush();

    // Update totalPages if we wrote a new page
    if (pageId >= totalPages)
    {
        totalPages = pageId + 1;
    }
}

uint32_t Pager::getNewPageId()
{
    uint32_t newId = totalPages;
    totalPages++;
    
    // Allocate and initialize an empty page at the end of the file
    char empty[PAGE_SIZE];
    std::memset(empty, 0, PAGE_SIZE);
    writePage(newId, empty);
    return newId;
}

uint32_t Pager::getTotalPages() const
{
    return totalPages;
}

void Pager::flush()
{
    if (file.is_open())
    {
        file.flush();
    }
}

void Pager::clear()
{
    if (file.is_open())
    {
        file.close();
    }
    // Truncate the file to zero size
    std::ofstream truncate(filename, std::ios::binary | std::ios::trunc);
    truncate.close();
    // Reopen the file for regular read/write operations
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    totalPages = 0;
}
