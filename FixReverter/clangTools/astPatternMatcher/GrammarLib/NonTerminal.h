#pragma once
#include <string>
#include <vector>
#include "PatternToken.h"
using std::vector;
using std::string;
class NonTerminal : public PatternToken
{
 public:
  NonTerminal(string s);
  NonTerminal(string s, vector<PatternToken*> r);
  virtual bool isTerminal() override;
  void addRule(vector<PatternToken*> r);
  bool canBeLambda();
  unsigned int numRules();
  vector<PatternToken*> getRule(int r);
  void makeLambda();
  void addToFollowSet(vector<PatternToken*> s);
  vector<PatternToken*> getFollowSet();
  void addToUpdateList(PatternToken* t);
  virtual string toString() override;
  virtual void constructUpdateList() override;
 protected:
  vector<PatternToken*> followSet;
  vector<vector<PatternToken*> > rules;
  bool lambda;
  vector<PatternToken*> updateList;
};
