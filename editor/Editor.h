#pragma once
#include "Buffer.h"
#include <ncurses.h>

class Editor;
typedef std::shared_ptr<Editor> EditorPtr;

class Editor
{
  public:
    //construct with memory buffer and title
    Editor(const std::string& data, const std::string& title);

    //construct with file to open and edit
    Editor(const std::string& fileName);

    //ncurses thread draws on screen and exits when user exits correctly
    void start();
    void stopThread();
    void join();

  private:
    typedef std::shared_ptr<std::thread> ThreadPtr;

    //current state
    enum Mode
    { 
      INSERT,
      NORMAL,
      EXIT
    };

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    void deleteLine();
    void deleteLine(int i);

    void saveFile();

    void handleInsertModeInput(int c);
    void handleNormalModeInput(int c);

    void initializeTerminal();
    void drawThread();
    void handleInput(int);

    void printStatusLine(const std::string& status);
    void updateStatus(const std::string& append="");
    void printBuff();

    void updateMode(Mode mode);
    int getMode();

    bool executeCommand();

    static const std::string STATUS_VERSION;
    
    const std::string     _statusTitle;
    unsigned int          _screenColumns;
    unsigned int          _screenLines;

    Mode                  _mode;

    std::recursive_mutex  _mutex;

    //current cursor location
    unsigned int          _x;
    unsigned int          _y;
    std::string           _cmd;         //command entered by user
    unsigned int          _lowerbound;  //lowerbound on screen, if screen rows < lines in text
    BufferPtr             _pBuff;       //buffer object with text
    std::string           _fileName;    //file name oppened
    ThreadPtr             _drawThread;  //thread that handles user input and draws on screen
};
