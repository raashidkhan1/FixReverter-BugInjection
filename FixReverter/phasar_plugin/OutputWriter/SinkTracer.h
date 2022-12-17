#ifndef SINKTRACER_H_
#define SINKTRACER_H_

#include <unordered_set>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"

#include "DataHolders/AstTracedVariable.h"

const bool sinkInRange(std::shared_ptr<AstTracedVariable>, const llvm::Instruction *,
                      const psr::LLVMBasedICFG *);

const bool reachableInCG(const llvm::Function *, std::vector<const llvm::CallInst *>,
                        const psr::LLVMBasedICFG *);

#endif
