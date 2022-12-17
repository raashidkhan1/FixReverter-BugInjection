#include "Operation.h"

std::string Operation::toString() {

  std::ostringstream ss;
  ss << "[" << getType()
     << " index=" << index
     << " file=" << fileName
     << " line=" << line
     << " col=" << col;
  if (type == DELETE || type == REPLACE)
    ss << " lineEnd=" << lineEnd
       << " colEnd=" << colEnd;
  if (type == INSERT || type == REPLACE)
    ss << " str=" << str;
  ss << "]";

  return ss.str();
}

std::string Operation::getType() {
  switch (type) {
  case DELETE:
    return "Delete";
    break;
  case INSERT:
    return "Insert";
    break;
  case REPLACE:
    return "Replace";
  }
}
