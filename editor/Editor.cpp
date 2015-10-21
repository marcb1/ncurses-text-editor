#include "Editor.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include "../helpers.h"

using namespace std;

const std::string Editor::STATUS_VERSION = "1.0";

Editor::Editor(const std::string& data, const std::string& title):
  upstatus(true),
  _statusTitle(title),
  _mode(NORMAL),
  _x(0),
  _y(0),
  cmd(""),
  lowerbound(0),
  raiseflag(false),
  _status("Normal Mode"),
  buff(new Buffer())
{
  buff->buildBuffer(data, '\n');
}

Editor::Editor(const std::string& fileName):
  upstatus(true),
  _statusTitle(fileName),
  _mode(NORMAL),
  _x(0),
  _y(0),
  cmd(""),
  lowerbound(0),
  raiseflag(false),
  _status("Normal Mode"),
  buff(new Buffer())
{
  buff->buildBuffer(fileName);
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


void Editor::updateMode(int mode)
{
  _mutex.lock();
  mode = mode;
  _mutex.unlock();
}
void Editor::stopThread()
{
  updateMode(EXIT);
}

void Editor::updateStatus()
{
  std::stringstream status;
  switch(_mode)
  {
    case NORMAL:
      if(cmd.empty())
        status << _statusTitle << "\t" << STATUS_VERSION;
      else
        status << cmd;
      break;
    case INSERT:
      status << "Insert Mode";
      break;
    case EXIT:
      status << "Exiting";
      break;
  }
  status << "\tCOL: " << _x  << "\tLINE: " << lowerbound + _y;
  status << " Y:" << _y << " SL: " << _screenLines << " LR" << lowerbound;
  _status = status.str();
}

void Editor::handleNormalModeInput(int c)
{
  assert(_mode == NORMAL);
  switch(c)
  {
    case KEY_LEFT:
    case 'l':
      moveLeft();
      break;
    case KEY_RIGHT:
    case 'h':
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
      //execute the current command
      execCmd();
      break;
    case 27:
      //esc/alt key clear command
      cmd.clear();
      break;
    case 127:
    case KEY_BACKSPACE:
    case KEY_DC:
      //remove last character from command
      if(!cmd.empty())
        cmd.erase(cmd.size()-1, 1);
      break;
    default:
      // Add character to command
      cmd.append(1, char(c));
      break;
  }
}


void Editor::handleInsertModeInput(int c)
{
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
        _x = buff->lines[_y-1].length();
        // Bring line down
        buff->lines[_y-1] += buff->lines[_y];
        // Delete the line
        deleteLine();
        moveUp();
      }
      else if(_x > 0)
      {
        buff->lines[_y].erase(--_x, 1);
      }
      break;
    case KEY_DC:
      // The Delete key
      if(_x == buff->lines[_y].length() && _y != buff->lines.size()-1)
      {
        // Bring line down
        buff->lines[_y] += buff->lines[_y+1];
        // Delete the line
        deleteLine(_y+1);
      }
      else
      {
        buff->lines[_y].erase(_x, 1);
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
      if(_x < buff->lines[_y+lowerbound].length()-1)
      {
        // Put rest of line on new line
        buff->insertLine(buff->lines[_y+lowerbound].substr(_x, buff->lines[_y+lowerbound].length()-_x), _y+1);
        // Remove that part of the line
        buff->lines[_y+lowerbound].erase(_x, buff->lines[_y+lowerbound].length()-_x);
      }
      else
        buff->insertLine("", _y+lowerbound+1);
      moveDown();
      break;
    case KEY_BTAB:
    case KEY_CTAB:
    case KEY_STAB:
    case KEY_CATAB:
    case 9:
      // The tab
      buff->lines[_y+lowerbound].insert(_x, 4, ' ');
      _x+=4;
      break;
    default:
      buff->lines[_y+lowerbound].insert(_x, 1, char(c));
      _x++;
      break;
  }

}
void Editor::deleteLine(int i)
{
  buff->removeLine(i);
}
void Editor::printStatusLine()
{
  if(raiseflag)
    attron(A_BOLD);
  attron(A_REVERSE);
  mvprintw(_screenLines-1, 0, _status.c_str());
  clrtoeol();
  if(raiseflag)
    attroff(A_BOLD);
  attroff(A_REVERSE);
}

void Editor::saveFile()
{
  bool ret = buff->saveToFile(_fileName);
  if(ret)
  {
    _status = "Saved to file!";
  }
  else
  {
    _status = "Error: Cannot open file for writing!";
  }
}

bool Editor::execCmd()
{
  if(cmd == ":q")
  {
    _mode = EXIT;
  }
  else if(cmd == ":w")
  {
    _mode = EXIT;
    saveFile();
  }

  cmd = "";                       // Reset command buffer
  return true;                    // Returns if command has executed successfully
}


void Editor::drawThread()
{
  initializeTerminal();
  while(_mode != EXIT)
  {
    if(upstatus)
    {
      updateStatus();
      printStatusLine();
    }
    printBuff();
    //blocks on input
    int input = getch();
    handleInput(input);
    usleep(50);
  }
  //stop things and let object destruct
  refresh();
  endwin();
}

//These functions work correctly
void Editor::handleInput(int c)
{
  upstatus = true;
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
  buff->removeLine(_y);
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
  if(_x+1 < _screenColumns && _x+1 <= buff->lines[_y+lowerbound].length())
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
  else if(_y == 0 && lowerbound > 0)
  {
    //wrap up
    lowerbound--;
  }
  else
  {
    //can't move up
    return;
  }
  //we can move up, so update x and move
  if(_x >= buff->lines[_y+lowerbound].length())
    _x = buff->lines[_y+lowerbound].length();
  move(_y, _x);
}

void Editor::moveDown()
{
  //_screenLines = status bar
  if(_y < _screenLines-2 && _y+1 < buff->lines.size())
  {
    _y++;
  }
  //+1 since y starts at 0. if buffer contains one line: 0+0+1 < 1
  else if(lowerbound+_y+1 < buff->lines.size())
  {
    raiseflag = true;
    lowerbound++;
  }
  else
  {
    //can't move down
    return;
  }
  if(_x >= buff->lines[_y].length())
    _x = buff->lines[_y].length();
  move(_y, _x);
}

void Editor::printBuff()
{
  unsigned int linesToPrint = 0;
  //_screenLines = status bar
  for(unsigned int i = lowerbound; linesToPrint < _screenLines-1; i++, linesToPrint++)
  {
    if(i >= buff->size())
    {
    }
    else
    {
      mvprintw(linesToPrint, 0, buff->lines[i].c_str());
    }
    clrtoeol();
  }
  move(_y, _x);
}
