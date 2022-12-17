#ifndef SEXTFLOWFUNCTION_H_
#define SEXTFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct SExtFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::SExtInst *SExtInst;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  SExtFlowFunction(
  GlobalDataHolder *GlobalData,
  const llvm::SExtInst *SExtInst,
  psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), SExtInst(SExtInst), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    if (source->as<MyFlowFact>()->isZero()) {
      return {source};
    }
    auto SourceValue = source->as<MyFlowFact>()->get().value();
    logFromToFlow(llvm::outs(), SourceValue, SExtInst, "SExt");
    auto PointerOperand = SExtInst->getOperand(0);
    if (SourceValue == PointerOperand) {
      // access tainted memory by load
      updateTaintPaths(std::make_shared<ValueNode>(SExtInst), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] SExt flow: connected value operand");
      return FFManager.getOrCreateFlowFacts(SourceValue, SExtInst);
    } else {
      return {source};
    }
  }
};

#endif
