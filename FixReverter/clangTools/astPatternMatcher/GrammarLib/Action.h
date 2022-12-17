#pragma once
#include <string>
#include <vector>
#include "PatternToken.h"
using std::vector;
using std::string;

class Action : public PatternToken
{
 public:
  Action() {beginSetComplete = true;}
  virtual bool isTerminal() override;
  virtual void constructUpdateList() override;
  virtual bool isAction() override;
  virtual bool isArray();
  virtual bool isID();
  virtual bool isName();
};
