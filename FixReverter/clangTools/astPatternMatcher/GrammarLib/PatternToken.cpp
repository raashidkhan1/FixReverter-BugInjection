#include "PatternToken.h"

PatternToken::PatternToken()
{
  id = "";
}

PatternToken::PatternToken(string name)
{
  id = name;
  beginSetComplete = false;
}

PatternToken::~PatternToken() {}

bool PatternToken::isThisPatternToken(string s)
{
  return id == s;
}

void PatternToken::addToBeginSet(vector<PatternToken*> s)
{
  for (unsigned int i = 0; i < s.size(); ++i)
    {
      if(std::find(beginSet.begin(), beginSet.end(), s[i]) == beginSet.end())
        beginSet.push_back(s[i]);
    }
}

bool PatternToken::isBeginSetComplete()
{
  return beginSetComplete;
}

vector<PatternToken*> PatternToken::getBeginSet()
{
  return beginSet;
}

void PatternToken::finishedBeginSet()
{
  beginSetComplete = true;
}

string PatternToken::getId()
{
  return id;
}

bool PatternToken::isAction()
{
  return false;
}
