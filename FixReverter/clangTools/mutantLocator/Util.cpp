#include "Util.h"
#include "utils.h"

string getPadding(string start, string end, int def = 1)
{
  string startLine, endLine;

  startLine = start.substr(start.find_first_of(":") + 1);
  endLine = end.substr(end.find_first_of(":") + 1);
  
  startLine = startLine.substr(0, startLine.find_first_of(":"));
  endLine = endLine.substr(0, endLine.find_first_of(":"));

  int s, e;
  std::istringstream(startLine) >> s;
  std::istringstream(endLine) >> e;
  e -= def;
  string ret = "";
  while(e >= s)
    {
      ret += "@";
      s++;
    }
  return ret;
}

json getInsertOp(int index, string startLoc, string content, int offset) {
  json op;
  op["operator"] = "insert";
  op["index"] = index;

  auto elems = stringSplit(startLoc, ':');
  op["file"] = elems[0];
  op["start"]["line"] = stoi(elems[1]);
  op["start"]["col"] = stoi(elems[2]) + offset;
  op["content"] = content;

  return op;
}

json getReplaceOp(int index, string startLoc, string endLoc, string content, int offset) {
  json op;
  op["operator"] = "replace";
  op["index"] = index;

  auto elems = stringSplit(startLoc, ':');
  op["file"] = elems[0];
  op["start"]["line"] = stoi(elems[1]);
  op["start"]["col"] = stoi(elems[2]);
  op["content"] = content;

  elems = stringSplit(endLoc, ':');
  op["end"]["line"] = stoi(elems[1]);
  op["end"]["col"] = stoi(elems[2]) + offset;

  return op;
}
