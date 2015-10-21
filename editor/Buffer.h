#pragma once

#include <ncurses.h>
#include <vector>
#include <string>
#include "../helpers.h"


class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;

class Buffer
{
  public:
    typedef std::vector<std::string>  LineSet;
    typedef LineSet::const_iterator   LineSetConstIterator;
    typedef LineSet::iterator         LineSetIterator;

    LineSet lines;

    Buffer();

    void buildBuffer(const std::string& data, char delim);
    void buildBuffer(const std::string& fileName);

    bool saveToFile(const std::string& fileName= "untitled");

    void insertLine(const std::string& line, int loc);
    void appendLine(const std::string& line);
    void removeLine(int);

    size_t size() const;

  private:
    void removeTabs(std::string& line);
};

