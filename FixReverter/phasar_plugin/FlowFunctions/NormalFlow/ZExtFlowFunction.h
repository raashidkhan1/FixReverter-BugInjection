#ifndef ZEXTFLOWFUNCTION_H_
#define ZEXTFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct ZExtFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::ZExtInst *ZExtInst;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  ZExtFlowFunction(
    GlobalDataHolder *GlobalData,
    const llvm::ZExtInst *ZExtInst,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), ZExtInst(ZExtInst), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    if (source->as<MyFlowFact>()->isZero()) {
      return {source};
    }
    auto SourceValue = source->as<MyFlowFact>()->get().value();
    logFromToFlow(llvm::outs(), SourceValue, ZExtInst, "ZExt");
    auto PointerOperand = ZExtInst->getOperand(0);
    if (SourceValue == PointerOperand) {
      // access tainted memory by load
      updateTaintPaths(std::make_shared<ValueNode>(ZExtInst), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] ZExt flow: connected value operand");
      return FFManager.getOrCreateFlowFacts(SourceValue, ZExtInst);
    } else {
      return {source};
    }
  }
};

#endif
