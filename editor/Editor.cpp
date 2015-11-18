#include "Editor.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include "helpers.h"

const std::string Editor::STATUS_VERSION = "1.0";

Editor::Editor(const std::string& data, const std::string& title):
    _statusTitle(title),
    _mode(NORMAL),
    _x(0),
    _y(0),
    _cmd(""),
    _lowerbound(0),
    _pBuff(new Buffer())
{
    _pBuff->buildBuffer(data, '\n');
}

Editor::Editor(const std::string& fileName):
    _statusTitle(fileName),
    _mode(NORMAL),
    _x(0),
    _y(0),
    _cmd(""),
    _lowerbound(0),
    _pBuff(new Buffer())
{
    _pBuff->buildBuffer(fileName);
}

int Editor::getMode()
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    return _mode;
}
void Editor::start()
{
    if(!_drawThread)
    {
        _drawThread = ThreadPtr(new std::thread(&Editor::drawThread, this));
    }
    else
    {
        std::cerr << "Thread already running" << std::endl;
    }
}

void Editor::join()
{
    _drawThread->join();
}

void Editor::updateMode(Mode mode)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    _mode = mode;
}
void Editor::stopThread()
{
    updateMode(EXIT);
}

//update string that will be drawn on status bar
void Editor::updateStatus(const std::string& append)
{
    std::stringstream status;
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    switch(_mode)
    {
        case NORMAL:
            if(_cmd.empty())
                status << _statusTitle << "\t" << STATUS_VERSION;
            else
                status << _cmd;
            break;
        case INSERT:
            status << "Insert Mode";
            break;
        case EXIT:
            status << "Exiting";
            break;
    }
    status << "\tCOL: " << _x  << "\tLINE: " << _lowerbound + _y;
    status << " Y:" << _y << " SL: " << _screenLines << " LR" << _lowerbound;
    status << append;
    printStatusLine(status.str());
}

void Editor::handleNormalModeCommand(int c)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    assert(_mode == NORMAL);
    assert(!_cmd.empty());

    switch(c)
    {
        case KEY_LEFT:
            moveLeft();
            break;
        case KEY_RIGHT:
            moveRight();
            break;
        case KEY_UP:
            moveUp();
            break;
        case KEY_DOWN:
            moveDown();
            break;
        case KEY_ENTER:
        case 10:
            executeCommand();
            break;
        case 27:
            //esc/alt key clear command
            _cmd.clear();
            break;
        case 127:
        case KEY_BACKSPACE:
        case KEY_DC:
            //remove last character from command
            if(!_cmd.empty())
            {
                _cmd.erase(_cmd.size()-1, 1);
            }
            break;
        default:
            // Add character to command
            _cmd.append(1, char(c));
            break;
    }
}

void Editor::handleNormalMode(int c)
{
    switch(c)
    {
        case KEY_LEFT:
        case 'h':
            moveLeft();
            break;
        case KEY_RIGHT:
        case 'l':
            moveRight();
            break;
        case KEY_UP:
        case 'k':
            moveUp();
            break;
        case KEY_DOWN:
        case 'j':
            moveDown();
            break;
        case 'i':
            _mode = INSERT;
            break;
        case KEY_ENTER:
        case 10:
            break;
        case 27:
            //esc/alt key clear command
            break;
        case 127:
        case KEY_BACKSPACE:
        case KEY_DC:
            break;
        default:
            // Add character to command
            _cmd.append(1, char(c));
            break;
    }
}

void Editor::handleNormalModeInput(int c)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    assert(_mode == NORMAL);
    if(!_cmd.empty())
    {
        handleNormalModeCommand(c);
    }
    else
    {
        handleNormalMode(c);
    }
}


void Editor::handleInsertModeInput(int c)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    switch(c)
    {
        case 27:
            // Escape/Alt key
            _mode = NORMAL;
            break;
        case 127:
        case KEY_BACKSPACE:
            // The Backspace
            if(_x == 0 && _y > 0)
            {
                _x = _pBuff->lines[_y-1].length();
                // Bring line down
                _pBuff->lines[_y-1] += _pBuff->lines[_y];
                // Delete the line
                deleteLine();
                moveUp();
            }
            else if(_x > 0)
            {
                _pBuff->lines[_y].erase(--_x, 1);
            }
            break;
        case KEY_DC:
            // The Delete key
            if(_x == _pBuff->lines[_y].length() && _y != _pBuff->lines.size()-1)
            {
                // Bring line down
                _pBuff->lines[_y] += _pBuff->lines[_y+1];
                // Delete the line
                deleteLine(_y+1);
            }
            else
            {
                _pBuff->lines[_y].erase(_x, 1);
            }
            break;
        case KEY_LEFT:
            moveLeft();
            break;
        case KEY_RIGHT:
            moveRight();
            break;
        case KEY_UP:
            moveUp();
            break;
        case KEY_DOWN:
            moveDown();
            break;
        case KEY_ENTER:
        case 10:
            // Bring rest of line down
            if(_x < _pBuff->lines[_y+_lowerbound].length()-1)
            {
                // Put rest of line on new line
                _pBuff->insertLine(_pBuff->lines[_y+_lowerbound].substr(_x, _pBuff->lines[_y+_lowerbound].length()-_x), _y+1);
                // Remove that part of the line
                _pBuff->lines[_y+_lowerbound].erase(_x, _pBuff->lines[_y+_lowerbound].length()-_x);
            }
            else
                _pBuff->insertLine("", _y+_lowerbound+1);
            moveDown();
            break;
        case KEY_BTAB:
        case KEY_CTAB:
        case KEY_STAB:
        case KEY_CATAB:
        case 9:
            // The tab
            _pBuff->lines[_y+_lowerbound].insert(_x, 4, ' ');
            _x+=4;
            break;
        default:
            _pBuff->lines[_y+_lowerbound].insert(_x, 1, char(c));
            _x++;
            break;
    }

}
void Editor::deleteLine(int i)
{
    _pBuff->removeLine(i);
}
void Editor::printStatusLine(const std::string& status)
{
    attron(A_REVERSE);
    mvprintw(_screenLines-1, 0, status.c_str());
    clrtoeol();
    attroff(A_REVERSE);
}

void Editor::saveFile()
{
    bool ret = _pBuff->saveToFile(_fileName);
    if(ret)
    {
        updateStatus("Saved to file: ");
    }
    else
    {
        updateStatus( "Error: Cannot open file for writing!");
    }
}

bool Editor::executeCommand()
{
    bool ret(false);
    if(_cmd == ":q")
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        ret = true;
        _mode = EXIT;
    }
    else if(_cmd == ":w")
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        ret = true;
        _mode = EXIT;
        saveFile();
    }
    else if(_cmd == ":x")
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        ret = true;
        _mode = EXIT;
    }
    else if(_cmd.find(":exec") == 0)
    {
        refresh();
        std::vector<std::string> arguments;
        splitString(_cmd, ' ', arguments);
        if(arguments[1] == "ssh")
        {
            refresh();
            endwin();
            int ret = execlp("sshpass", "sshpass", "-p", _pBuff->lines[_y].c_str(), "ssh", 
                    "-oStrictHostKeyChecking=no", arguments[2].c_str(), NULL);
            if(ret < 0)
            {
                std::cerr << "exec failed! returning to editor..." << std::endl;
                printErrno();
                sleep(2);
            }
        }
    }
    // Reset command buffer
    _cmd.clear();
    return ret;
}


void Editor::drawThread()
{
    initializeTerminal();
    int mode = getMode();
    while(mode != EXIT)
    {
        //print status line first, since getch blocks
        updateStatus();
        printBuff();
        //blocks on input
        int input = getch();
        handleInput(input);
        mode = getMode();
    }
    //stop things and let object destruct
    refresh();
    endwin();
}

//These functions work correctly
void Editor::handleInput(int c)
{
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    switch(_mode)
    {
        case NORMAL:
            handleNormalModeInput(c);
            break;
        case INSERT:
            handleInsertModeInput(c);
            break;
        default:
            break;
    }
}

void Editor::deleteLine()
{
    _pBuff->removeLine(_y);
}

void Editor::initializeTerminal()
{
    initscr();
    noecho();
    //disable line buffering
    cbreak();
    //capture special keys
    keypad(stdscr, true);
    assert(COLS > 0);
    _screenColumns = COLS;
    assert(LINES > 0);
    _screenLines = LINES;
}

void Editor::moveLeft()
{
    if(_x > 0)
    {
        _x--;
        move(_y, _x);
    }
}

void Editor::moveRight()
{
    if(_x+1 < _screenColumns && _x+1 <= _pBuff->lines[_y+_lowerbound].length())
    {
        _x++;
        move(_y, _x);
    }
}

void Editor::moveUp()
{
    if(_y > 0)
    {
        _y--;
    }
    else if(_y == 0 && _lowerbound > 0)
    {
        //wrap up
        _lowerbound--;
    }
    else
    {
        //can't move up
        return;
    }
    //we can move up, so update x and move
    if(_x >= _pBuff->lines[_y+_lowerbound].length())
        _x = _pBuff->lines[_y+_lowerbound].length();
    move(_y, _x);
}

void Editor::moveDown()
{
    //_screenLines = status bar
    if(_y < _screenLines-2 && _y+1 < _pBuff->lines.size())
    {
        //move cursor down
        _y++;
    }
    //+1 since y starts at 0. if buffer contains one line: 0+0+1 < 1
    else if(_lowerbound+_y+1 < _pBuff->lines.size())
    {
        //keep cursor at bottom of the screen, but scroll down in the text file
        _lowerbound++;
    }
    else
    {
        //can't move down
        return;
    }
    if(_x >= _pBuff->lines[_y].length())
        _x = _pBuff->lines[_y].length();
    move(_y, _x);
}

std::string Editor::getBufferAsString() const
{
    std::stringstream ss;
    _pBuff->saveToString(ss);
    return ss.str();
}

void Editor::printBuff()
{
    unsigned int linesToPrint = 0;
    //_screenLines = status bar
    for(unsigned int i = _lowerbound; linesToPrint < _screenLines-1; i++, linesToPrint++)
    {
        if(i >= _pBuff->size())
        {
        }
        else
        {
            mvprintw(linesToPrint, 0, _pBuff->lines[i].c_str());
        }
        clrtoeol();
    }
    move(_y, _x);
}
