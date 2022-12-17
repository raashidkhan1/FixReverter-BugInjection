#ifndef ASTPATTERNPARSER_H
#define ASTPATTERNPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>

#include "DataHolders/AstTracedVariable.h"
#include "nlohmann/json.hpp"

typedef std::map<const std::string, std::set<std::shared_ptr<AstTracedVariable>>> tracemap;

nlohmann::json getApm(const std::string);

std::vector<std::shared_ptr<AstTracedVariable>> parseAstPattern (const nlohmann::json);

tracemap getDecToTraceMap(std::vector<std::shared_ptr<AstTracedVariable>>);

#endif
