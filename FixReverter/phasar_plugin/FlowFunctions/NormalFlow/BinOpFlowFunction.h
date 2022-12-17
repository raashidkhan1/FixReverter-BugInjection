#ifndef BINOPFLOWFUNCTION_H_
#define BINOPFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct BinOpFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::BinaryOperator *BinOP;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  BinOpFlowFunction(
    GlobalDataHolder *GlobalData,
    const llvm::BinaryOperator *BinOP,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), BinOP(BinOP), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    if (source->as<MyFlowFact>()->isZero()) {
      return {source};
    }
    auto SourceValue = source->as<MyFlowFact>()->get().value();
    logFromToFlow(llvm::outs(), SourceValue, BinOP, "binop");

    unsigned numOperands = BinOP->getNumOperands();
    for (unsigned i = 0; i < numOperands; ++i) {
      if (SourceValue == BinOP->getOperand(i)) {
        updateTaintPaths(std::make_shared<ValueNode>(BinOP), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
        logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] tainted by binary operator:", BinOP);
        return FFManager.getOrCreateFlowFacts(BinOP, SourceValue);
      }
    }
    return {source};
    
  }
};

#endif
