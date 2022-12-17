#ifndef UTILFUNCS_H
#define UTILFUNCS_H

#include <set>
#include <map>
#include <string>
#include "llvm/IR/Value.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "Configs.h"
#include "DataHolders/TaintNode.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h"

std::string getEnvVar(const std::string);

void updateTaintPaths(std::shared_ptr<TaintNode>, std::shared_ptr<TaintNode>, 
                        std::unordered_map<std::shared_ptr<TaintNode>,
                        std::unordered_set<std::shared_ptr<TaintNode>, TaintNodeHash, TaintNodeEquality>,
                        TaintNodeHash, TaintNodeEquality> &);

#endif

