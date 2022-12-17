#include "OutputWriter.h"
#include "SinkTracer.h"
#include "DataHolders/AstTracedVariable.h"
#include "DataHolders/TaintNode.h"

#include <llvm/Support/raw_ostream.h>

using namespace std;
using json = nlohmann::json;
namespace fs = boost::filesystem;


void WriteOutput(const string OutPath,
                  GlobalDataHolder *GlobalData,
                  json ApmJsonData,
                  const psr::LLVMBasedICFG *ICFG) {
  json out;
  unordered_set<shared_ptr<AstTracedVariable>, 
                AstTracedVarHash, AstTracedVarEquality> CrashedTracedVars;
  vector<pair<shared_ptr<AstTracedVariable>, const llvm::Instruction *>> CrashedCondExeTracedVars;

  int logInterval = (GlobalData->Leaks.size() < 20) ? 1 : GlobalData->Leaks.size()/20;
  int logTimes = 0;
  int count = 0;

  for (auto Leak : GlobalData->Leaks) {
    if (++count > logTimes * logInterval) {
      cout << "tracing taint sources has finished " << logTimes++ * 5 << "%" << endl;
    }
    
    queue<shared_ptr<TaintNode>> Queue;
    auto LeakKey = make_shared<ValueNode>(Leak);
    Queue.push(LeakKey);
    unordered_set<shared_ptr<TaintNode>,
                                TaintNodeHash, TaintNodeEquality> Visited = {LeakKey};
    unordered_set<shared_ptr<AstTracedVariable>, 
                AstTracedVarHash, AstTracedVarEquality> TaintedSources;
    while(!Queue.empty()) {
      auto TaintKeyCurr = Queue.front();
      Queue.pop();
      if (GlobalData->DecToTraceMap.count(TaintKeyCurr)) {
        for (auto t : GlobalData->DecToTraceMap[TaintKeyCurr])
          TaintedSources.insert(t);
      }
      if (GlobalData->TaintPaths.count(TaintKeyCurr)) {
        for (auto Pre : GlobalData->TaintPaths[TaintKeyCurr]) {
          if (!Visited.count(Pre)) {
            Queue.push(Pre);
            Visited.insert(Pre);
          }
        }
      }
    }
    for (auto t : TaintedSources) {
      if (t->isCondExe())
        CrashedCondExeTracedVars.push_back(make_pair(t, Leak));
      else
        CrashedTracedVars.insert(t);
    }
  }

  for (auto pair : CrashedCondExeTracedVars) {
    if (!CrashedTracedVars.count(pair.first)) {
      if (sinkInRange(pair.first, pair.second, ICFG))
        CrashedTracedVars.insert(pair.first);
    }
  }

  unordered_set<int> CrashedJsonIndices;
  for (auto t : CrashedTracedVars) {
    const int JsonIndex = t->getJsonIndex();
    const int iid = t->getIdentifierID();
    CrashedJsonIndices.insert(JsonIndex);
    ApmJsonData[JsonIndex]["ids"][iid]["crash"] = true;
  }

// post-process conditional execute patterns
  

  for (auto i : CrashedJsonIndices)
    out.push_back(ApmJsonData[i]);

  cout << "number of patterns reported: " << CrashedJsonIndices.size() << endl;

  ofstream File(OutPath);
  File << std::setw(2) << out << std::endl;
}

void WriteStat(const string OutPath,
                GlobalDataHolder *GlobalData,
                json ApmJsonData) {
  json out;
  unordered_set<int> ReachedIndices;

  for (auto pair : GlobalData->DecToTraceMap)
    for (auto t : pair.second)
      ReachedIndices.insert(t->getJsonIndex());

  cout << "number of patterns reached: " << ReachedIndices.size() << endl;

  for (auto i : ReachedIndices)
    out.push_back(ApmJsonData[i]);

  ofstream File (OutPath);
  File << std::setw(2) << out << std::endl;
}
