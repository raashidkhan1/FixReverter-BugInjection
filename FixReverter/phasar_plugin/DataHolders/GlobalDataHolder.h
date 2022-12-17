#ifndef GLOBALDATAHOLDER_H_
#define GLOBALDATAHOLDER_H_

#include <unordered_map>
#include <unordered_set>

#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h"

#include "AstTracedVariable.h"
#include "FuncDataHolder.h"
#include "TaintNode.h"

struct GlobalDataHolder
{
  std::unordered_set<const llvm::Instruction *> Leaks;
  // map from a taint source to the set of DetectedSource that has it as source
  std::unordered_map<std::shared_ptr<TaintNode>, 
                     std::unordered_set<std::shared_ptr<AstTracedVariable>,
                        AstTracedVarHash, AstTracedVarEquality>,
                     TaintNodeHash, TaintNodeEquality> DecToTraceMap;
  // records the value that taints the key value
  std::unordered_map<std::shared_ptr<TaintNode>,
                     std::unordered_set<std::shared_ptr<TaintNode>,
                        TaintNodeHash, TaintNodeEquality>,
                     TaintNodeHash, TaintNodeEquality> TaintPaths;
  std::unordered_map<const llvm::Function *, std::shared_ptr<FuncDataHolder>> MethodData;
  long FlowCounter = 0;
};

#endif
