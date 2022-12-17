#ifndef ALLOCAFLOWFUNCTION_H_
#define ALLOCAFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct AllocaFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::AllocaInst *Alloca;
  psr::PointsToInfo<DependenceAnalyzer::v_t, DependenceAnalyzer::n_t> *PT;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  AllocaFlowFunction(
    GlobalDataHolder *GlobalData,
    const llvm::AllocaInst *Alloca,
    psr::PointsToInfo<DependenceAnalyzer::v_t, DependenceAnalyzer::n_t> *PT,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), Alloca(Alloca), PT(PT), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    auto FlowSource = source->as<MyFlowFact>();
    if (FlowSource->isZero()) {
      auto TaintKey = std::make_shared<ValueNode>(Alloca);
      if (GlobalData->DecToTraceMap.count(TaintKey)) {
        std::set<DependenceAnalyzer::d_t> ret = {source};
        bool added = false;
        for (auto TraceVar : GlobalData->DecToTraceMap[TaintKey]) {
          if (TraceVar->isField()) {
            int64_t offset = TraceVar->getFieldOffset();
            std::shared_ptr<FieldNode> FieldBasedTaint = NULL;
            if (auto AllocatedType = llvm::dyn_cast<llvm::PointerType>(Alloca->getAllocatedType())) {
              FieldBasedTaint = std::make_shared<FieldNode>(AllocatedType, offset);
            } else {
              FieldBasedTaint = std::make_shared<FieldNode>(Alloca->getType(), offset);
            }
            GlobalData->DecToTraceMap[FieldBasedTaint].insert(TraceVar);
            updateTaintPaths(FieldBasedTaint, FieldBasedTaint, GlobalData->TaintPaths);
            logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] tainted field base by: ", Alloca);
          } else if (!added) {
            added = true;
            logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] marked as taint source: ", Alloca);
            ret.insert(FFManager.getOrCreateFlowFact(Alloca));
            for(auto User : Alloca->users()) {
              if (auto Load = llvm::dyn_cast<llvm::LoadInst>(User)) {
                //taint all aliases
                auto PTS = PT->getPointsToSet(Load);
                for (const auto *Alias : *PTS) {
                  if (auto AliasInst = llvm::dyn_cast<llvm::Instruction>(Alias)) {
                    if (Alloca->getParent()->getParent() == AliasInst->getParent()->getParent()) {
                      ret.insert(FFManager.getOrCreateFlowFact(Alias));
                      updateTaintPaths(std::make_shared<ValueNode>(Alias), TaintKey, GlobalData->TaintPaths);
                      logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] tainted as source alias: ", Alias);
                    }
                  }
                }
                break;
              }
            }
          }
        }
        return ret;
      }
    }
    return {source};
  }
};


#endif
