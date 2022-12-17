#pragma once
#include <string>
#include <vector>
#include "Action.h"
using std::vector;
using std::string;

class IDAction : public Action
{
 public:
  IDAction(string s);
  virtual bool isID() override;
  virtual string toString() override;
};
