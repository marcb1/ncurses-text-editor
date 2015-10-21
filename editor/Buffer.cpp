#include "Buffer.h"

Buffer::Buffer()
{
}

size_t Buffer::size() const
{
  return lines.size();
}

bool Buffer::saveToFile(const std::string& fileName)
{
  bool ret(false);
  std::ofstream f(fileName.c_str());
  if(f.is_open())
  {
    LineSetConstIterator iter;
    iter = lines.begin();
    for(; iter != lines.end(); iter++)
    {
      f << *iter << std::endl;
    }
    ret = true;
  }
  f.close();
  return ret;
}

void Buffer::buildBuffer(const std::string& fileName)
{
  std::ifstream input(fileName);
  for(std::string line; getline(input, line); )
  {
    appendLine(line);
  }
  input.close();
}

void Buffer::buildBuffer(const std::string& data, char delim)
{
  std::vector<std::string> lines;
  splitString(data, delim, lines);

  std::vector<std::string>::const_iterator iter;
  iter = lines.begin();
  for(; iter != lines.end(); iter++)
  {
    appendLine(*iter);
  }
}

void Buffer::removeTabs(std::string& line)
{
    size_t tab = line.find("\t");
    if(tab == std::string::npos)
        return;
    else
        return removeTabs(line.replace(tab, 1, "    "));
}

void Buffer::insertLine(const std::string& line, int x)
{
//    line = remTabs(line);
    lines.insert(lines.begin()+x, line);
}

void Buffer::appendLine(const std::string& line)
{
 //   line = remTabs(line);
    lines.push_back(line);
}

void Buffer::removeLine(int i)
{
    lines.erase(lines.begin()+i);
}
