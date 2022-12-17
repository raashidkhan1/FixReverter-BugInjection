#ifndef DEPENDENCEANALYZER_H_
#define DEPENDENCEANALYZER_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
#include "phasar/DB/ProjectIRDB.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/IfdsIde/FlowFunctions.h"
#include "phasar/PhasarLLVM/DataFlowSolver/IfdsIde/LLVMZeroValue.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h"
#include "phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h"
#include "phasar/PhasarLLVM/Plugins/Interfaces/IfdsIde/IFDSTabulationProblemPlugin.h"

#include "nlohmann/json.hpp"
#include "DataHolders/GlobalDataHolder.h"
#include "DataHolders/AstTracedVariable.h"
#include "AstPatternParser/AstPatternParser.h"


namespace psr {
class ProjectIRDB;
class LLVMTypeHierarchy;
class LLVMBasedICFG;
class LLVMPointsToInfo;
} // namespace psr

struct MyFlowFact : public psr::FlowFactWrapper<const llvm::Value *> {

  using FlowFactWrapper::FlowFactWrapper;

  void print(std::ostream &OS,
             const llvm::Value *const &NonZeroFact) const override {
    OS << psr::llvmIRToShortString(NonZeroFact) << '\n';
  }
};

class DependenceAnalyzer : public psr::IFDSTabulationProblemPlugin {
private:
  psr::FlowFactManager<MyFlowFact> FFManager;
  tracemap FileToTraceMap;
  nlohmann::json ApmJsonData;
  GlobalDataHolder *GlobalData = new GlobalDataHolder();

  std::string OutResPath;
  std::string OutStatPath;

public:
  DependenceAnalyzer(const psr::ProjectIRDB *IRDB, const psr::LLVMTypeHierarchy *TH,
                const psr::LLVMBasedICFG *ICF, psr::LLVMPointsToInfo *PT,
                std::set<std::string> EntryPoints);

  ~DependenceAnalyzer() override;

  const psr::FlowFact *createZeroValue() const override;

  FlowFunctionPtrType
  getNormalFlowFunction(n_t curr,
                        n_t succ) override;

  FlowFunctionPtrType getCallFlowFunction(n_t callStmt,
                                       f_t destMthd) override;

  FlowFunctionPtrType
  getRetFlowFunction(n_t callSite,
                     f_t calleeMthd,
                     n_t exitStmt,
                     n_t retSite) override;

  FlowFunctionPtrType
  getCallToRetFlowFunction(n_t callSite,
                           n_t retSite,
                           std::set<f_t> callees) override;

  FlowFunctionPtrType
  getSummaryFlowFunction(n_t callStmt,
                         f_t destMthd) override;

  psr::InitialSeeds<n_t, d_t, l_t> initialSeeds() override;

  void genMethodData(f_t);
};

extern "C" std::unique_ptr<psr::IFDSTabulationProblemPlugin>
makeDependenceAnalyzer(const psr::ProjectIRDB *IRDB,
                  const psr::LLVMTypeHierarchy *TH,
                  const psr::LLVMBasedICFG *ICF, psr::LLVMPointsToInfo *PT,
                  std::set<std::string> EntryPoints);

#endif
