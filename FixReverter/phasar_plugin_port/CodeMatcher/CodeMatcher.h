#ifndef SOURCEMATCHER_H_
#define SOURCEMATCHER_H_

#include <string>
#include <set>
#include <unordered_set>
#include <map>

#include "DataHolders/AstTracedVariable.h"
#include "DataHolders/FuncDataHolder.h"

std::shared_ptr<FuncDataHolder> matchSourceCode(const llvm::Function* F,
                            std::map<const std::string,
                            std::set<std::shared_ptr<AstTracedVariable>>> &InputDecMap);

#endif
