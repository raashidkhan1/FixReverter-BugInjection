#ifndef GETELEPTRFLOWFUNCTION_H_
#define GETELEPTRFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct GetElePtrFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::GetElementPtrInst *GetElePtrInst;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  GetElePtrFlowFunction(
    GlobalDataHolder *GlobalData,
    const llvm::GetElementPtrInst *GetElePtrInst,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), GetElePtrInst(GetElePtrInst), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    auto PointerOperand = GetElePtrInst->getPointerOperand();
    unsigned NumIndices = GetElePtrInst->getNumIndices();
    std::shared_ptr<FieldNode> FieldBasedTaint = NULL;
    auto GEPTaintValue = std::make_shared<ValueNode>(GetElePtrInst);
    std::set<DependenceAnalyzer::d_t> toGenerate = {source};

    auto GEPFlowFact = FFManager.getOrCreateFlowFact(GetElePtrInst);
    if (NumIndices > 1) {
      auto PointerTy = llvm::dyn_cast<llvm::PointerType>(GetElePtrInst->getPointerOperandType());
      if (auto ConstInt = llvm::dyn_cast<llvm::ConstantInt>(GetElePtrInst->getOperand(2))) {
        int64_t offset = ConstInt->getSExtValue();
        FieldBasedTaint = std::make_shared<FieldNode>(PointerTy, offset);
        if (FieldBasedTaint && GlobalData->TaintPaths.count(FieldBasedTaint)) {
          updateTaintPaths(GEPTaintValue, FieldBasedTaint, GlobalData->TaintPaths);
          logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] tainted by field base: ", GetElePtrInst);
          toGenerate.insert(GEPFlowFact);
        }
      }
    }

    if (source->as<MyFlowFact>()->isZero()) {
      return toGenerate;
    }

    auto SourceValue = source->as<MyFlowFact>()->get().value();
    auto SourceTaintValue = std::make_shared<ValueNode>(SourceValue);
    logFromToFlow(llvm::outs(), SourceValue, GetElePtrInst, "GEP");

    if (SourceValue == PointerOperand || SourceValue == GetElePtrInst->getOperand(1) || (NumIndices > 1 && SourceValue == GetElePtrInst->getOperand(2))) {
      updateTaintPaths(GEPTaintValue, SourceTaintValue, GlobalData->TaintPaths);
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] getElePtr flow: connected value operand");

      if (FieldBasedTaint) {
        updateTaintPaths(FieldBasedTaint, GEPTaintValue, GlobalData->TaintPaths);
        logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] GEP is tainted, taint field base", GetElePtrInst);
      }
      toGenerate.insert(GEPFlowFact);
    }
    return toGenerate;
  }
};

#endif
