#include "Terminal.h"

Terminal::Terminal(string s) : PatternToken(s)
{
  beginSet.push_back((PatternToken*)this);
  beginSetComplete = true;
}

bool Terminal::isTerminal ()
{
  return true;
}

string Terminal::toString()
{
  return "Terminal:" + id;
}

void Terminal::constructUpdateList()
{
  return;
}
