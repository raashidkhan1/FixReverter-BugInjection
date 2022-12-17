#pragma once
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include "PatternToken.h"
#include "Terminal.h"
#include "NonTerminal.h"
#include "Action.h"
#include "State.h"
#include "Grammar.h"
#include "json.hpp"

using std::pair;
using std::vector;
using std::string;
using json = nlohmann::json;

struct StackFrame
{
  StackFrame(State* s, json js)
  {
    state = s;
    j = js;
  }
  State* state;
  json j;
};

struct ParseData
{
  vector<StackFrame> stack;
  vector<pair<string, json> > namedObjs;
  vector<pair<string, vector<json> > > arrayObjs;
  vector<json> tokenOrder;
  bool valid = true;
};

class Validator
{
 public:
  Validator(bool parseInfo);
  Validator(Grammar* g, bool parseInfo);
  
  // rule of five
  ~Validator();
  Validator(const Validator& other);
  Validator(Validator&& other);
  Validator& operator=(const Validator& other);
  Validator& operator=(Validator&& other);
  
  void processToken(string t, json j, int pos);
  void processTokenRecursive(string t, json j, int pos, ParseData* pd);
  string getPattern();
  vector<json> getJsons();
  vector<json> getTokenOrder();
  bool isValid();
  bool allParsesValid();
  int size();
  vector<ParseData*> popSuccessfulParses();
  void pushSuccessfulParses(vector<ParseData*> toAppend);
  string getVal(ParseData* p, vector<PatternToken*> v, int spot);
  json getJson(ParseData* p, vector<PatternToken*> v, int spot);

 private:
  Grammar* grammar;
  vector<ParseData*> parseQueue;
  ParseData* mainParse;
  bool valid;
  bool parseInfo;
  void checkFinalActions();
};
