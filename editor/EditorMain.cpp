#include "Editor.h"

int main(int argc, char** argv)
{
  if(argc != 2)
  {
    std::cerr << "Usage: ./editor_main file_name" << std::endl;
    return 1;
  }
  EditorPtr ed(new Editor(argv[1]));
  ed->start();
  ed->join();
  std::string buff = ed->getBufferAsString();
  std::cout << buff << std::endl;
  return 0;
}
