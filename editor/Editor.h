#pragma once
#include "Buffer.h"
#include <ncurses.h>

class Editor;
typedef std::shared_ptr<Editor> EditorPtr;

class Editor
{
  public:
    bool upstatus;

    //construct with memory buffer and title
    Editor(const std::string& data, const std::string& title);

    //construct with file to open and edit
    Editor(const std::string& fileName);

    //ncurses thread draw on screen and exit when use exits correctly
    void start();
    void stopThread();
    void join();

    void handleInput(int);
    void printBuff();
    void printStatusLine();
    void updateStatus();

  private:
    typedef std::shared_ptr<std::thread> ThreadPtr;

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    void deleteLine();
    void deleteLine(int i);

    void saveFile();

    void handleInsertModeInput(int c);
    void handleNormalModeInput(int c);

    void drawThread();

    void initializeTerminal();

    void updateMode(int mode);

    //figure out what these do
    std::string tos(int);

    bool execCmd();

    //current state
    enum State
    { 
      INSERT,
      NORMAL,
      EXIT
    };

    //editing a file, or a buffer from memory
    enum EditorMode
    {
      FILE_EDIT,
      BUFFER_EDIT
    };

    static const std::string STATUS_VERSION;
    
    const std::string   _statusTitle;
    unsigned int        _screenColumns;
    unsigned int        _screenLines;

    State               _mode;

    std::mutex          _mutex;

    //current cursor location
    unsigned int        _x;
    unsigned int        _y;
    std::string         _cmd;         //command entered by user
    unsigned int        _lowerbound;  //lowerbound on screen, if screen rows < lines in text
    std::string         _status;      //status bar string
    BufferPtr           _pBuff;       //buffer object with text
    std::string         _fileName;    //file name openned
    ThreadPtr           _drawThread;  //thread that handles user input and draws on screen
};
