#ifndef CALLFLOWFUNCTION_H
#define CALLFLOWFUNCTION_H

#include "Utils/Utils.h"
#include "Utils/Logs.h"

struct CallFlowFunction : psr::FlowFunction<DependenceAnalyzer::d_t> {
  const llvm::CallBase *callSite;
  const llvm::Function *destMthd;
  GlobalDataHolder *GlobalData;
  std::unordered_set<const llvm::Value *> GlobalVars;
  std::vector<const llvm::Value *> actuals;
  std::vector<const llvm::Value *> formals;
  psr::FlowFactManager<MyFlowFact> &FFManager;
  std::function<bool(const llvm::Value *)> predicate;

  CallFlowFunction(const llvm::CallBase *callSite,
    const llvm::Function *destMthd,
    GlobalDataHolder *GlobalData,
    std::unordered_set<const llvm::Value *> GlobalVars,
    psr::FlowFactManager<MyFlowFact> &FFManager,
    std::function<bool(const llvm::Value *)> predicate =
      [](const llvm::Value *) { return true; }) : callSite(callSite), destMthd(destMthd), GlobalData(GlobalData), GlobalVars(GlobalVars), FFManager(FFManager), predicate(predicate) {
    // Set up the actual parameters
    for (unsigned idx = 0; idx < callSite->getNumArgOperands(); ++idx) {
      actuals.push_back(callSite->getArgOperand(idx));
    }
    // Set up the formal parameters
    for (unsigned idx = 0; idx < destMthd->arg_size(); ++idx) {
      formals.push_back(psr::getNthFunctionArgument(destMthd, idx));
    }
  };
  std::set<DependenceAnalyzer::d_t> computeTargets(DependenceAnalyzer::d_t source) override {
    auto FlowSource = source->as<MyFlowFact>();

    #if DEBUG_MODE == 1 && FLOW_COUNTER == 1
    GlobalData->FlowCounter ++;
    std::cout << "FlowCounter = " << GlobalData->FlowCounter << "\n";
    #endif

    std::set<DependenceAnalyzer::d_t> res;
    if (!FlowSource->isZero()) {
      auto SourceValue = FlowSource->get().value();
      auto SourceTaintValue = std::make_shared<ValueNode>(SourceValue);

      if (destMthd)
        logFromToFlow(llvm::outs(), SourceValue, destMthd->getName(), "call");
      else
        logFromToFlow(llvm::outs(), SourceValue, "NULL", "call");
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] call flow: SourceValue is not zero");
      // Handle C-style varargs functions
      if (destMthd->isVarArg()) {
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] call flow: handle c-style varags functions");
        // Map actual parameter into corresponding formal parameter.
        for (unsigned idx = 0; idx < actuals.size(); ++idx) {
          if (SourceValue == actuals[idx] && predicate(actuals[idx])) {
            if (idx >= destMthd->arg_size() && !destMthd->isDeclaration()) {
              // Over-approximate by trying to add the
              //   alloca [1 x %struct.__va_list_tag], align 16
              // to the results
              // find the allocated %struct.__va_list_tag and generate it
              for (auto &BB : *destMthd) {
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
                      updateTaintPaths(std::make_shared<ValueNode>(Alloc), SourceTaintValue, GlobalData->TaintPaths);
                      logFromToFlow(llvm::outs(), SourceValue, Alloc, "call");
                      res.insert(FFManager.getOrCreateFlowFact(Alloc));
                    }
                  }
                }
              }
            } else {
              if (idx < formals.size()) {
                updateTaintPaths(std::make_shared<ValueNode>(formals[idx]), SourceTaintValue, GlobalData->TaintPaths);
                logFromToFlow(llvm::outs(), SourceValue, formals[idx], "call");
                res.insert(FFManager.getOrCreateFlowFact(formals[idx])); // corresponding formal
              }
            }
          }
        }
        return res;
      } else {
        // Handle ordinary case
        // Map actual parameter into corresponding formal parameter.
        logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] call flow: handle ordinary case");
        if (actuals.size() == formals.size()) {
          for (unsigned idx = 0; idx < actuals.size(); ++idx) {
            predicate(actuals[idx]);
            if (SourceValue == actuals[idx] && predicate(actuals[idx])) {
              updateTaintPaths(std::make_shared<ValueNode>(formals[idx]), SourceTaintValue, GlobalData->TaintPaths);
              logFromToFlow(llvm::outs(), SourceValue, formals[idx], "call");
              res.insert(FFManager.getOrCreateFlowFact(formals[idx])); // corresponding formal
            }
          }
        } else {
          logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] call flow: formal and actual parameter sizes don't match, skip this case");
        }
        return res;
      }
    } else {
      // generate zerovalue and all global variables
      logGeneralInfo(llvm::outs(), "Analyzer - [DEBUG] call flow: generate tainted global variables");
      for (auto Global : GlobalVars)
        res.insert(FFManager.getOrCreateFlowFact(Global));
      res.insert(source);
      return {source};
    }
  }
};

#endif
