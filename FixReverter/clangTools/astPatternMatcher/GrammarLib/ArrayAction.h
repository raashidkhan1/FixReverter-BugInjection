#pragma once
#include <string>
#include <vector>
#include "Action.h"
using std::vector;
using std::string;

class ArrayAction : public Action
{
 public:
  ArrayAction(string s);
  virtual bool isArray() override;
  virtual string toString() override;
};
