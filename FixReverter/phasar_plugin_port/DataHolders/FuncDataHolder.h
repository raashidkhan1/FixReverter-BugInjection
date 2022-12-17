#ifndef FUNCDATAHOLDER_H_
#define FUNCDATAHOLDER_H_

#include <unordered_map>
#include <unordered_set>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include "AstTracedVariable.h"
#include "TaintNode.h"

struct FuncDataHolder
{
  std::unordered_map<std::shared_ptr<TaintNode>,
                    std::unordered_set<std::shared_ptr<AstTracedVariable>,
                        AstTracedVarHash, AstTracedVarEquality>,
                    TaintNodeHash, TaintNodeEquality> DecToTraceMap;
  std::unordered_set<const llvm::Value *> DecAllocs;
  std::unordered_set<const llvm::Value *> GlobalVars;
};

#endif
