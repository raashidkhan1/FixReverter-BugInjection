#pragma once

#include <string>
#include <sstream>

enum OperationType { DELETE, INSERT, REPLACE };

struct Operation {
public:
  std::string fileName;
  OperationType type;

  int index;
  int line, col;
  int lineEnd, colEnd; // insert and delete only
  std::string str; // replace only
  std::string getType();
  std::string toString();
};
