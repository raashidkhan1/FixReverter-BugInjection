#ifndef SOURCEINFO_H_
#define SOURCEINFO_H_

#include <string>

typedef struct SourceInfo {
  const std::string file;
  const int line;
  const int col;
} SourceInfo;

#endif
