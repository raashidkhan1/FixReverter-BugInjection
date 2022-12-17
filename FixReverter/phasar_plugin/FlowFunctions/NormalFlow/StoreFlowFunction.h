#ifndef STOREFLOWFUNCTION_H_
#define STOREFLOWFUNCTION_H_

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct StoreFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
private:
  GlobalDataHolder *GlobalData;
  const llvm::StoreInst *store;
  psr::FlowFactManager<MyFlowFact> &FFManager;

public:
  StoreFlowFunction(GlobalDataHolder *GlobalData,
    const llvm::StoreInst *s,
    psr::FlowFactManager<MyFlowFact> &FFManager) : GlobalData(GlobalData), store(s), FFManager(FFManager) {}

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    if (source->as<MyFlowFact>()->isZero()) {
      return {source};
    }
    auto SourceValue = source->as<MyFlowFact>()->get().value();
    logFromToFlow(llvm::outs(), SourceValue, store, "store");

    std::set<DependenceAnalyzer::d_t> toGenerate = {source};
    auto ValueOperand = store->getValueOperand();
    auto PointerOperand = store->getPointerOperand();

    if (ValueOperand == SourceValue) {
      logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] stores tainted value into:", PointerOperand);
      auto PointerOperandTaint = std::make_shared<ValueNode>(PointerOperand);
      updateTaintPaths(PointerOperandTaint, std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
      toGenerate.insert(FFManager.getOrCreateFlowFact(PointerOperand));

      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] normal flow: connected pointer operand by StoreInst");

      if (auto GetElePtrInst = llvm::dyn_cast<llvm::GetElementPtrInst>(PointerOperand)) {
        unsigned NumIndices = GetElePtrInst->getNumIndices();
        if (NumIndices > 1) {
          auto PointerTy = llvm::dyn_cast<llvm::PointerType>(GetElePtrInst->getPointerOperandType());
          if (auto ConstInt = llvm::dyn_cast<llvm::ConstantInt>(GetElePtrInst->getOperand(2))) {
            int64_t offset = ConstInt->getSExtValue();
            std::shared_ptr<FieldNode> FieldBasedTaint = std::make_shared<FieldNode>(PointerTy, offset);
            updateTaintPaths(FieldBasedTaint, PointerOperandTaint, GlobalData->TaintPaths);
            logGeneralInfoWithValue(llvm::outs(), "Analyzer - [DEBUG] storing tainted value into GEP, taint field base", GetElePtrInst);
          }
        }
      }

      return toGenerate;
    // this is a source storeInst
    } else if (ValueOperand != SourceValue &&
                PointerOperand == SourceValue) {
      // handle LHS dereference
      if (llvm::isa<llvm::LoadInst>(SourceValue)) {
        GlobalData->Leaks.insert(store);
        updateTaintPaths(std::make_shared<ValueNode>(store), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] store flow: reach a sink, stop further analysis");
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
          GlobalData->Leaks.insert(store);
          updateTaintPaths(std::make_shared<ValueNode>(store), std::make_shared<ValueNode>(SourceValue), GlobalData->TaintPaths);
          logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] store flow: reach a sink, stop further analysis");
          if (!taintedByFieldBase)
            return {};
        }
        return {source};
      } else {
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] normal flow: storing to declare value, keep source");
        return {source};
      }
    } else {
      // keep as-is
      return {source};
    }
  }
};

#endif
