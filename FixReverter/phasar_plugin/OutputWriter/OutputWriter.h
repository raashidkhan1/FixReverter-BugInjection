#ifndef OUTPUTWRITER_H
#define OUTPUTWRITER_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <boost/filesystem.hpp>
#include "nlohmann/json.hpp"

#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"

#include "DataHolders/GlobalDataHolder.h"

void WriteOutput (const std::string OutPath,
                    GlobalDataHolder *GlobaltData,
                    nlohmann::json ApmJsonData,
                    const psr::LLVMBasedICFG *);

void WriteStat (const std::string OutPath,
                  GlobalDataHolder *GlobaltData,
                  nlohmann::json ApmJsonData);

#endif
