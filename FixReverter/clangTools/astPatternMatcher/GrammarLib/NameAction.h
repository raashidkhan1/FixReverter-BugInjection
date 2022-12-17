#pragma once
#include <string>
#include <vector>
#include "Action.h"
using std::vector;
using std::string;

class NameAction : public Action
{
 public:
  NameAction(string s);
  virtual string toString() override;
  virtual bool isName() override;
  string getName();
};
