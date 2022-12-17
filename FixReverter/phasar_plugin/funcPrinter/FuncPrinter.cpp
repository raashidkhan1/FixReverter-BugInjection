#include <iostream>

#include <phasar/DB/ProjectIRDB.h>
#include <phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h>
#include <phasar/PhasarLLVM/DataFlowSolver/IfdsIde/FlowFunctions.h>
#include <phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h>
#include <phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include "FuncPrinter.h"

using namespace std;
using namespace psr;

// Factory function that is used to create an instance by the Phasar framework.
unique_ptr<IFDSTabulationProblemPlugin>
makeFuncPrinter(const ProjectIRDB *IRDB, const LLVMTypeHierarchy *TH,
                  const LLVMBasedICFG *ICF, LLVMPointsToInfo *PT,
                  std::set<std::string> EntryPoints) {
  return unique_ptr<IFDSTabulationProblemPlugin>(
      new FuncPrinter(IRDB, TH, ICF, PT, EntryPoints));
}

// Is executed on plug-in load and has to register this plug-in to Phasar.
__attribute__((constructor)) void init() {
  cout << "init - FuncPrinter\n";
  IFDSTabulationProblemPluginFactory["FuncPrinter"] = &makeFuncPrinter;
}

// Is executed on unload, can be used to unregister the plug-in.
__attribute__((destructor)) void fini() { cout << "fini - FuncPrinter\n"; }

FuncPrinter::FuncPrinter(const ProjectIRDB *IRDB,
                             const LLVMTypeHierarchy *TH,
                             const LLVMBasedICFG *ICF, LLVMPointsToInfo *PT,
                             std::set<std::string> EntryPoints)
    : IFDSTabulationProblemPlugin(IRDB, TH, ICF, PT, EntryPoints) {
  auto Functions = ICF->getAllFunctions();
  for (auto F : Functions) {
    if (auto MN = F->getMetadata(llvm::LLVMContext::MD_dbg)) {
      if (auto DIScope = llvm::dyn_cast<llvm::DIScope>(MN)) {
        std::string PathStr;
        PathStr = DIScope->getDirectory().str();
        PathStr += "/";
        PathStr += DIScope->getFilename().str();
        PathStr += ":";
        llvm::outs() << PathStr << F->getName() << "\n";
      }
    }
  }
  std::exit(0);
}

 const FlowFact *FuncPrinter::createZeroValue() const {
   return ZeroValue;
 }

FuncPrinter::FlowFunctionPtrType
FuncPrinter::getNormalFlowFunction(const llvm::Instruction *curr,
                                     const llvm::Instruction *succ) {
  cout << "FuncPrinter::getNormalFlowFunction()\n";
  // TODO: Must be implemented to propagate tainted values through the program.
  // Tainted values may spread through loads and stores (llvm::LoadInst and
  // llvm::StoreInst). Important memberfunctions are getPointerOperand() and
  // getPointerOperand()/ getValueOperand() respectively.
  return Identity<const FlowFact *>::getInstance();
}

FuncPrinter::FlowFunctionPtrType
FuncPrinter::getCallFlowFunction(const llvm::Instruction *callStmt,
                                   const llvm::Function *destMthd) {
  cout << "FuncPrinter::getCallFlowFunction()\n";
  // TODO: Must be modeled to perform parameter passing:
  // actuals at caller-side must be mapped into formals at callee-side.
  // LLVM distinguishes between a ordinary function call and a function call
  // that might throw (llvm::CallInst, llvm::InvokeInst). To be able to easily
  // inspect both, a variable of type llvm::ImmutableCallSite may be
  // constructed using 'callStmt'.
  // Important: getCallFlowFunction() can also be used in combination with
  // getCallToRetFlowFunction() in order to model a function's effect without
  // actually following call targets. This must be used to model sources and
  // sinks of the taint analysis. It works by killing all flow facts at the
  // call-site and generating the desired facts within the
  // getCallToRetFlowFunction.
  return Identity<const FlowFact *>::getInstance();
}

FuncPrinter::FlowFunctionPtrType FuncPrinter::getRetFlowFunction(
    const llvm::Instruction *callSite, const llvm::Function *calleeMthd,
    const llvm::Instruction *exitStmt, const llvm::Instruction *retSite) {
  cout << "FuncPrinter::getRetFlowFunction()\n";
  // TODO: Must be modeled to map the return value back into the caller's
  // context. When dealing with pointer parameters one must also map the
  // formals at callee-side back into the actuals at caller-side. All other
  // facts that do not influence the caller must be killed.
  // 'callSite' can be handled by using llvm::ImmutableCallSite, 'exitStmt' is
  // the function's return instruction - llvm::ReturnInst may be used.
  // The 'retSite' is - in case of LLVM - the call-site and it is possible
  // to wrap it into an llvm::ImmutableCallSite.
  return Identity<const FlowFact *>::getInstance();
}

FuncPrinter::FlowFunctionPtrType
FuncPrinter::getCallToRetFlowFunction(const llvm::Instruction *callSite,
                                        const llvm::Instruction *retSite,
                                        set<const llvm::Function *> callees) {
  cout << "FuncPrinter::getCallToRetFlowFunction()\n";
  // TODO: Use in combination with getCallFlowFunction to model the effects of
  // source and sink functions. It most analyses flow facts can be passed as
  // identity.
  return Identity<const FlowFact *>::getInstance();
}

// May be used to model function calls to libc or llvm.intrinsic functions
// for which no implementation is accessible. If nullptr is returned it applies
// identity on all flow facts that are present.
FuncPrinter::FlowFunctionPtrType
FuncPrinter::getSummaryFlowFunction(const llvm::Instruction *callStmt,
                                      const llvm::Function *destMthd) {
  cout << "FuncPrinter::getSummaryFlowFunction()\n";
  return nullptr;
}

// Return(s) set(s) of flow fact(s) that hold(s) initially at a corresponding
// statement. The analysis will start at these instructions and propagate the
// flow facts according to the analysis description.
psr::InitialSeeds<FuncPrinter::n_t, FuncPrinter::d_t,
             FuncPrinter::l_t>
FuncPrinter::initialSeeds() {
  cout << "FuncPrinter::initialSeeds()\n";
  psr::InitialSeeds<FuncPrinter::n_t, FuncPrinter::d_t,
               FuncPrinter::l_t> Seeds;
  auto EntryPoints = getEntryPoints();
  for (const auto &EntryPoint : EntryPoints) {
    if (const auto *F = IRDB->getFunctionDefinition(EntryPoint)) {
      Seeds.addSeed(&ICF->getFunction(EntryPoint)->front().front(),
                  getZeroValue());
    } else {
      cout << "Could not retrieve function '" << EntryPoint
           << "' --> Function does not exist or is declaration only.\n";
    }
  }
  return Seeds;
}
