#ifndef RETFLOWFUNCTION_H
#define RETFLOWFUNCTION_H

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct RetFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
  GlobalDataHolder *GlobalData;
  const llvm::CallBase *callSite;
  const llvm::Function *calleeMthd;
  const llvm::ReturnInst *exitStmt;
  psr::FlowFactManager<MyFlowFact> &FFManager;
  std::vector<const llvm::Value *> actuals;
  std::vector<const llvm::Value *> formals;
  std::function<bool(const llvm::Value *)> paramPredicate;
  std::function<bool(const llvm::Function *)> returnPredicate;

  RetFlowFunction(
  GlobalDataHolder *GlobalData,
  const llvm::CallBase *cs, const llvm::Function *calleeMthd,
  const llvm::Instruction *exitstmt,
  psr::FlowFactManager<MyFlowFact> &FFManager,
  std::function<bool(const llvm::Value *)> paramPredicate = [](const llvm::Value *) { return true; },
  std::function<bool(const llvm::Function *)> returnPredicate = [](const llvm::Function *) { return true; })
  : GlobalData(GlobalData), callSite(cs), calleeMthd(calleeMthd),
    exitStmt(llvm::dyn_cast<llvm::ReturnInst>(exitstmt)),
    FFManager(FFManager),
    paramPredicate(paramPredicate), returnPredicate(returnPredicate) {
    // Set up the actual parameters
    for (unsigned idx = 0; idx < callSite->getNumArgOperands(); ++idx) {
      actuals.push_back(callSite->getArgOperand(idx));
    }
    // Set up the formal parameters
    for (unsigned idx = 0; idx < calleeMthd->arg_size(); ++idx) {
      formals.push_back(psr::getNthFunctionArgument(calleeMthd, idx));
    }
  }

  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    auto FlowSource = source->as<MyFlowFact>();

    if (!FlowSource->isZero()) {
      auto SourceValue = FlowSource->get().value();
      auto SourceTaintValue = std::make_shared<ValueNode>(SourceValue);
      logFromToFlow(llvm::outs(), SourceValue, callSite, "return");
      std::set<DependenceAnalyzer::d_t> res;
      // Handle C-style varargs functions
      if (calleeMthd->isVarArg() && !calleeMthd->isDeclaration()) {
        const llvm::Instruction *AllocVarArg;
        // Find the allocation of %struct.__va_list_tag
        for (auto &BB : *calleeMthd) {
          for (auto &I : BB) {
            if (auto Alloc = llvm::dyn_cast<llvm::AllocaInst>(&I)) {
              if (Alloc->getAllocatedType()->isArrayTy() &&
                  Alloc->getAllocatedType()->getArrayNumElements() > 0 &&
                  Alloc->getAllocatedType()
                      ->getArrayElementType()
                      ->isStructTy() &&
                  Alloc->getAllocatedType()
                          ->getArrayElementType()
                          ->getStructName() == "struct.__va_list_tag") {
                AllocVarArg = Alloc;
                // TODO break out this nested loop earlier (without goto ;-)
              }
            }
          }
        }
        // Generate the varargs things by using an over-approximation
        if (SourceValue == AllocVarArg) {
          logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] return flow: generate the varargs things by using an over-approximation");
          for (unsigned idx = formals.size(); idx < actuals.size(); ++idx) {
            updateTaintPaths(std::make_shared<ValueNode>(actuals[idx]), SourceTaintValue, GlobalData->TaintPaths);
            logFromToFlow(llvm::outs(), SourceValue, actuals[idx], "return");
            res.insert(FFManager.getOrCreateFlowFact(actuals[idx]));
          }
        }
      }
      // Handle ordinary case
      // Map formal parameter into corresponding actual parameter.
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] return flow: handle ordinary case");
      for (unsigned idx = 0; idx < formals.size(); ++idx) {
        if (SourceValue == formals[idx] && paramPredicate(formals[idx])) {
          updateTaintPaths(std::make_shared<ValueNode>(actuals[idx]), SourceTaintValue, GlobalData->TaintPaths);
          logFromToFlow(llvm::outs(), SourceValue, actuals[idx], "return");
          res.insert(FFManager.getOrCreateFlowFact(actuals[idx])); // corresponding actual
        }
      }
      // Collect return value facts
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] return flow: collect return value facts");
      if (SourceValue == exitStmt->getReturnValue() && returnPredicate(calleeMthd)) {
        updateTaintPaths(std::make_shared<ValueNode>(callSite), SourceTaintValue, GlobalData->TaintPaths);
        logFromToFlow(llvm::outs(), SourceValue, callSite, "return");
        res.insert(FFManager.getOrCreateFlowFact(callSite));
      }
      return res;
    } else {
      return {source};
    }
  }
};

#endif
