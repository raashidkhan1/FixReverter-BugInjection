#include "SinkTracer.h"

#include <queue>

const bool sinkInRange(std::shared_ptr<AstTracedVariable> tracedVar, const llvm::Instruction *sink,
                      const psr::LLVMBasedICFG *ICFG) {
  auto rangeFunc = tracedVar->getRangeFunc();
  auto sinkFunc = sink->getParent()->getParent();
  auto InstsInRage = tracedVar->getInstsInRange();

  // if sink is in same function as the conditional, simply check if sink falls in range
  if (rangeFunc == sinkFunc)
    return InstsInRage.count(sink);

  std::vector<const llvm::CallInst *> CallInsts;
  for (auto Inst : InstsInRage) {
    if (auto CallInst = llvm::dyn_cast<llvm::CallInst>(Inst)) {
      CallInsts.push_back(CallInst);    
    }
  }

  return reachableInCG(sinkFunc, CallInsts, ICFG);
}

const bool reachableInCG(const llvm::Function *targetFunc, std::vector<const llvm::CallInst *> CallInsts,
                        const psr::LLVMBasedICFG *ICFG) {
  std::queue<const llvm::Function *> Queue;
  std::unordered_set<const llvm::Function *> visited;

  for (auto CallInst : CallInsts) {
    for (auto Callee : ICFG->getCalleesOfCallAt(CallInst)) {
      Queue.push(Callee);
      visited.insert(Callee);
    }
  }

  while (!Queue.empty()) {
    auto Func = Queue.front();
    Queue.pop();

    if (Func == targetFunc)
      return true;

    for (auto CallInst : ICFG->getCallsFromWithin(Func)) {
      for (auto Callee : ICFG->getCalleesOfCallAt(CallInst)) {
        if (!visited.count(Callee))
          Queue.push(Callee);
          visited.insert(Callee);
      }
    }
  }
  return false;
}
