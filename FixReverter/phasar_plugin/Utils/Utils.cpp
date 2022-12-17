#include "Utils.h"

std::string getEnvVar(const std::string key) {
  char *val = getenv(key.c_str());
  return val == NULL ? std::string("") : std::string(val);
}

void updateTaintPaths(std::shared_ptr<TaintNode> Target, std::shared_ptr<TaintNode> Source, 
    std::unordered_map<std::shared_ptr<TaintNode>,
    std::unordered_set<std::shared_ptr<TaintNode>, TaintNodeHash, TaintNodeEquality>,
    TaintNodeHash, TaintNodeEquality> &TaintPaths) {
  TaintPaths[Target].insert(Source);
}
