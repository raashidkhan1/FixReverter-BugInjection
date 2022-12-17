#ifndef LOADFLOWFUNCTION_H_
#define LOADFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct LoadFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::LoadInst *Load;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  LoadFlowFunction(
    GlobalDataHolder *GlobalData, 
    const llvm::LoadInst *L,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), Load(L), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    if (source->as<MyFlowFact>()->isZero()) {
      return {source};
    }
    auto SourceValue = source->as<MyFlowFact>()->get().value();
    logFromToFlow(llvm::outs(), SourceValue, Load, "load");
    if (SourceValue == Load->getPointerOperand()) {
      //handle RHS dereference
      if (llvm::isa<llvm::LoadInst>(SourceValue)) {
        GlobalData->Leaks.insert(Load);
        updateTaintPaths(std::make_shared<ValueNode>(Load), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] load flow: reach a sink, stop further analysis");
        return {};
      } else if (llvm::isa<llvm::GetElementPtrInst>(SourceValue)) {
        bool taintedByFlow = false;
        bool taintedByFieldBase = false;
        auto SourceNode = std::make_shared<ValueNode>(SourceValue);
        if (GlobalData->TaintPaths.count(SourceNode)) {
          for (auto Pre : GlobalData->TaintPaths[SourceNode]) {
            if (std::dynamic_pointer_cast<ValueNode>(Pre) != nullptr)
              taintedByFlow = true;
            else
              taintedByFieldBase = true;
          }
        }
        if (taintedByFlow) {
          GlobalData->Leaks.insert(Load);
          updateTaintPaths(std::make_shared<ValueNode>(Load), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
          logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] load flow: reach a sink, stop further analysis");
          if (!taintedByFieldBase)
            return {};
        }
        return {source};
      } else {
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] load flow: connected value operand");
        updateTaintPaths(std::make_shared<ValueNode>(Load), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
        return FFManager.getOrCreateFlowFacts(Load, SourceValue);
      }
    } else {
      return {source};
    }
  }
};

#endif
