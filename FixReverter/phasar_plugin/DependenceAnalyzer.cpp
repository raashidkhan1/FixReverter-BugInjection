#include <iostream>

#include "llvm/IR/InstrTypes.h"

#include "DependenceAnalyzer.h"
#include "Utils/Utils.h"
#include "Utils/Logs.h"
#include "FlowFunctions/NormalFlow/AllocaFlowFunction.h"
#include "FlowFunctions/NormalFlow/StoreFlowFunction.h"
#include "FlowFunctions/NormalFlow/LoadFlowFunction.h"
#include "FlowFunctions/NormalFlow/GetElePtrFlowFunction.h"
#include "FlowFunctions/NormalFlow/CastFlowFunction.h"
#include "FlowFunctions/NormalFlow/SExtFlowFunction.h"
#include "FlowFunctions/NormalFlow/ZExtFlowFunction.h"
#include "FlowFunctions/NormalFlow/BinOpFlowFunction.h"
#include "FlowFunctions/SummaryFlowFunction.h"
#include "FlowFunctions/CallFlowFunction.h"
#include "FlowFunctions/RetFlowFunction.h"
#include "CodeMatcher/CodeMatcher.h"
#include "OutputWriter/OutputWriter.h"

using namespace std;
using namespace psr;

short LOG_DEBUG_INFO;

// Factory function that is used to create an instance by the Phasar framework.
unique_ptr<IFDSTabulationProblemPlugin>
makeDependenceAnalyzer(const ProjectIRDB *IRDB, const LLVMTypeHierarchy *TH,
                  const LLVMBasedICFG *ICF, LLVMPointsToInfo *PT,
                  set<string> EntryPoints) {
  return unique_ptr<IFDSTabulationProblemPlugin>(
      new DependenceAnalyzer(IRDB, TH, ICF, PT, EntryPoints));
}

// Is executed on plug-in load and has to register this plug-in to Phasar.
__attribute__((constructor)) void init() {
  cout << "init - DependenceAnalyzer\n";
  IFDSTabulationProblemPluginFactory["DependenceAnalyzer"] = &makeDependenceAnalyzer;
}

// Is executed on unload, can be used to unregister the plug-in.
__attribute__((destructor)) void fini() {cout << "fini - DependenceAnalyzer\n";}

DependenceAnalyzer::DependenceAnalyzer(const ProjectIRDB *IRDB,
                             const LLVMTypeHierarchy *TH,
                             const LLVMBasedICFG *ICF, LLVMPointsToInfo *PT,
                             set<string> EntryPoints)
    : IFDSTabulationProblemPlugin(IRDB, TH, ICF, PT, EntryPoints) {
  ZeroValue = FFManager.getOrCreateZero();

  string ApmPathStr = getEnvVar("FIXREVERTER_DA_APM");
  string OutPathStr = getEnvVar("FIXREVERTER_DA_OUT");
  string LogEnabled = getEnvVar("FIXREVERTER_LOG_ENABLED");
  if (ApmPathStr.empty()) {
    llvm::errs() <<  "env variable FIXREVERTER_DA_APM must be set" << "\n";
    exit(0);
  }
  if (OutPathStr.empty()) {
    llvm::errs() <<  "env variable FIXREVERTER_DA_OUT must be set" << "\n";
    exit(0);
  }
  if (LogEnabled.empty()) {
    llvm::outs() <<  "taint logs will be suppressed" << "\n";
    LOG_DEBUG_INFO = 0;
  } else {
    llvm::outs() <<  "taint logs will be omitted" << "\n";
    LOG_DEBUG_INFO = 1;
  }

  boost::filesystem::path ApmPath(ApmPathStr);
  if (!boost::filesystem::is_regular_file(ApmPath)) {
    llvm::errs() <<  "apm file does not exist" << "\n";
    exit(0);
  }
  boost::filesystem::path OutPath(OutPathStr);
  boost::filesystem::create_directory(OutPath);

  OutResPath = (OutPath / boost::filesystem::path("dda.json")).string();
  OutStatPath = (OutPath / boost::filesystem::path("dda_reached.json")).string();

  ApmJsonData = getApm(ApmPathStr);
  auto AstTracedVars = parseAstPattern(ApmJsonData);
  FileToTraceMap = getDecToTraceMap(AstTracedVars);
}

DependenceAnalyzer::~DependenceAnalyzer() {
  WriteOutput(OutResPath, GlobalData, ApmJsonData, ICF);
  WriteStat(OutStatPath, GlobalData, ApmJsonData);

}

 DependenceAnalyzer::d_t DependenceAnalyzer::createZeroValue() const {
   return ZeroValue;
 }

DependenceAnalyzer::FlowFunctionPtrType
DependenceAnalyzer::getNormalFlowFunction(DependenceAnalyzer::n_t curr,
                                     DependenceAnalyzer::n_t succ) {
  if (auto Alloca = llvm::dyn_cast<llvm::AllocaInst>(curr))
    return make_shared<AllocaFlowFunction>(GlobalData, Alloca, PT, FFManager);
  
  if (auto Store = llvm::dyn_cast<llvm::StoreInst>(curr))
    return make_shared<StoreFlowFunction>(GlobalData, Store, FFManager);

  if (auto Load = llvm::dyn_cast<llvm::LoadInst>(curr))
    return make_shared<LoadFlowFunction>(GlobalData, Load, FFManager);

  if (auto GetElePtr = llvm::dyn_cast<llvm::GetElementPtrInst>(curr))
    return make_shared<GetElePtrFlowFunction>(GlobalData, GetElePtr, FFManager);

  if (auto Cast = llvm::dyn_cast<llvm::CastInst>(curr))
    return make_shared<CastFlowFunction>(GlobalData, Cast, FFManager);

  if (auto SExt = llvm::dyn_cast<llvm::SExtInst>(curr))
    return make_shared<SExtFlowFunction>(GlobalData, SExt, FFManager);

  if (auto ZExt = llvm::dyn_cast<llvm::ZExtInst>(curr))
    return make_shared<ZExtFlowFunction>(GlobalData, ZExt, FFManager);

  if (auto BinOp = llvm::dyn_cast<llvm::BinaryOperator>(curr))
    return make_shared<BinOpFlowFunction>(GlobalData, BinOp, FFManager);

  return Identity<DependenceAnalyzer::d_t>::getInstance();
}

DependenceAnalyzer::FlowFunctionPtrType
DependenceAnalyzer::getCallFlowFunction(DependenceAnalyzer::n_t callStmt,
                                   DependenceAnalyzer::f_t destMthd) {
  logFlowFunction(llvm::outs(), callStmt, "call");

  // kill all debug functions here and handle them in calltoret flow
  if (llvm::isa<llvm::DbgDeclareInst>(callStmt) || llvm::isa<llvm::DbgValueInst>(callStmt)) {
    return KillAll<DependenceAnalyzer::d_t>::getInstance();
  }

  // get set of source instructions from detector
  // TODO: remove duplicate codes
  genMethodData(destMthd);
  auto GlobalVars = GlobalData->MethodData[destMthd]->GlobalVars;

  // Map the actual into the formal parameters
  if (llvm::isa<llvm::CallInst>(callStmt) ||
      llvm::isa<llvm::InvokeInst>(callStmt)) {
    return make_shared<CallFlowFunction>(llvm::cast<llvm::CallBase>(callStmt), destMthd, GlobalData, GlobalVars, FFManager);
  }

  return Identity<DependenceAnalyzer::d_t>::getInstance();
}

DependenceAnalyzer::FlowFunctionPtrType DependenceAnalyzer::getRetFlowFunction(
    DependenceAnalyzer::n_t callSite, DependenceAnalyzer::f_t calleeMthd,
    DependenceAnalyzer::n_t exitStmt, DependenceAnalyzer::n_t retSite) {
  return make_shared<RetFlowFunction>(
      GlobalData, llvm::cast<llvm::CallBase>(callSite), calleeMthd, exitStmt, FFManager,
      [](const llvm::Value *formal) {
        return formal->getType()->isPointerTy();
      });
}

DependenceAnalyzer::FlowFunctionPtrType
DependenceAnalyzer::getCallToRetFlowFunction(DependenceAnalyzer::n_t callSite,
                                        DependenceAnalyzer::n_t retSite,
                                        set<DependenceAnalyzer::f_t > callees) {
  logFlowFunction(llvm::outs(), callSite, "call to ret");
  // pass everything as it is
  return Identity<DependenceAnalyzer::d_t>::getInstance();
}

// May be used to model function calls to libc or llvm.intrinsic functions
// for which no implementation is accessible. If nullptr is returned it applies
// identity on all flow facts that are present.
DependenceAnalyzer::FlowFunctionPtrType
DependenceAnalyzer::getSummaryFlowFunction(DependenceAnalyzer::n_t callStmt,
                                      DependenceAnalyzer::f_t destMthd) {
  vector<int> SinkIndex = {};

//  if (llvm::isa<llvm::MemCpyInst>(callStmt))
//    SinkIndex = {0, 1, 2};
  if (destMthd->getName() == "streq")
    SinkIndex = {1, 1};
  else if (destMthd->getName() == "strncmp")
    SinkIndex = {0, 1};
  else if (destMthd->getName() == "strcmp")
    SinkIndex = {0, 1};
  else if (destMthd->getName() == "strlen")
    SinkIndex = {0, 1};
  else if (destMthd->getName() == "fread")
    SinkIndex = {0, 3};
  else if (destMthd->getName() == "fgetc")
    SinkIndex = {0};
  else if (destMthd->getName() == "fseek")
    SinkIndex = {0};
  else if (destMthd->getName() == "fputc")
    SinkIndex = {0};
  else if (destMthd->getName() == "fwrite")
    SinkIndex = {3};
  else if (destMthd->getName() == "fclose")
    SinkIndex = {0};
  else if (destMthd->getName().find("llvm.ctlz") != string::npos)
    SinkIndex = {0};

  if (SinkIndex.empty()) {
    // Otherwise we indicate, that not special summary exists
    // and the solver thus calls the call flow function instead
    return nullptr;
  }
  return make_shared<SummaryFlowFunction>(GlobalData, callStmt, destMthd, SinkIndex, FFManager);
}

// Return(s) set(s) of flow fact(s) that hold(s) initially at a corresponding
// statement. The analysis will start at these instructions and propagate the
// flow facts according to the analysis description.
psr::InitialSeeds<DependenceAnalyzer::n_t, DependenceAnalyzer::d_t,
             DependenceAnalyzer::l_t>
DependenceAnalyzer::initialSeeds() {
  cout << "DependenceAnalyzer::initialSeeds()\n";
  psr::InitialSeeds<DependenceAnalyzer::n_t, DependenceAnalyzer::d_t,
               DependenceAnalyzer::l_t> Seeds;

  for (auto &EntryPoint : EntryPoints) {
    if (const auto *F = IRDB->getFunctionDefinition(EntryPoint)) {
      Seeds.addSeed(&ICF->getFunction(EntryPoint)->front().front(),
                  getZeroValue());
      // generate all global variables
      genMethodData(F);
      set<d_t> toGenerate;
      auto GlobalVars = GlobalData->MethodData[F]->GlobalVars;
      for (auto Global : GlobalVars)
        Seeds.addSeed(&ICF->getFunction(EntryPoint)->front().front(),
                    FFManager.getOrCreateFlowFact(Global));
      // generate zero value
    } else {
      cout << "Could not retrieve function '" << EntryPoint
           << "' --> Function does not exist or is declaration only.\n";
    }
  }
  return Seeds;
}

void DependenceAnalyzer::genMethodData(const llvm::Function* F) {
  // generates method data if haven't
  if (GlobalData->MethodData.count(F) == 0) {
    auto FuncData = matchSourceCode(F, FileToTraceMap);
    GlobalData->MethodData[F] = FuncData;
    GlobalData->DecToTraceMap.insert(FuncData->DecToTraceMap.begin(), FuncData->DecToTraceMap.end());
  }
}
