#pragma once
#include <string>
#include <vector>
#include <algorithm>
using std::vector;
using std::string;

class PatternToken
{
 public:
  PatternToken();
  PatternToken(string name);
  virtual ~PatternToken() = 0;
  virtual bool isTerminal() = 0;
  virtual string toString() = 0;
  string getId();
  bool isThisPatternToken(string s);
  void addToBeginSet(vector<PatternToken*> s);
  bool isBeginSetComplete();
  vector<PatternToken*> getBeginSet();
  void finishedBeginSet();
  virtual void constructUpdateList() = 0;
  virtual bool isAction();
 protected:
  string id;
  vector<PatternToken*> beginSet;
  bool beginSetComplete;
  
};
