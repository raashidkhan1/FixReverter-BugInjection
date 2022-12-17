#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <iterator>
#include <map>
#include "PatternToken.h"
#include "Terminal.h"
#include "NonTerminal.h"
using std::vector;
using std::string;
using std::map;
using std::pair;

struct Production
{
public:
  Production(NonTerminal* h, vector<PatternToken*> b, unsigned int p)
  {
    head = h;
    body = b;
    pos = p;
  }
  NonTerminal* head;
  vector<PatternToken*> body;
  unsigned int pos;
  bool eq(Production p)
  {
    return p.head == head && p.body == body && p.pos == pos;
  }
  string toString()
  {
    string ret = head->getId() + " -> ";
    for (unsigned int i = 0; i < body.size(); ++i)
      {
        if (pos == i)
          ret += ". ";
        ret += body[i]->getId() + " ";
      }
    if (pos == body.size())
      ret += ".";
    return ret;
  }
};

struct Reduction
{
public:
  Reduction(string s, int i, vector<PatternToken*> v, vector<PatternToken*> b)
  {
    retTok = s;
    numReduce = i;
    reduceSet = v;
    ruleBody = b;
  }
  string retTok;
  unsigned int numReduce;
  vector<PatternToken*> reduceSet;
  vector<PatternToken*> ruleBody;
  string toString()
  {
    string ret = retTok + " <- " + std::to_string(numReduce) + " on:";
    for (unsigned int i = 0; i < reduceSet.size(); ++i)
      ret += reduceSet[i]->getId() + " ";
    return ret;
  }
};

class State
{
 public:
  static int count;
  State(vector<PatternToken*>);
  string toString();
  bool isShift(string s);
  bool isReduction(string s);
  vector<Reduction> getReductions(string s);
  State* shift(string s);
  Reduction reduce(string s);
  bool isEndState();
  string getPattern();
  int getId();
  bool getConflicts();
  bool hasConflicts();
  vector<PatternToken*> getFinalRules();
 private:
  State(State* r, vector<Production> ps);
  int id;
  bool conflict;
  vector<Production> prods;
  vector<Reduction> reds;
  map<string,State*> paths;
  State* root;
  void createPaths();
  State* searchStates(vector<int>& v, vector<Production> p);
  string toString(vector<int>& v);
};
