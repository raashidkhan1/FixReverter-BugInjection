#pragma once
#include <string>
#include <vector>
#include "PatternToken.h"
using std::vector;
using std::string;

class Terminal : public PatternToken
{
 public:
  Terminal(string s);
  virtual bool isTerminal() override;
  virtual string toString() override;
  virtual void constructUpdateList() override;
 protected:
  
};
