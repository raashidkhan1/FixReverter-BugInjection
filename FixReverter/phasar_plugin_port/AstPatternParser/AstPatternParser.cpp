#include "AstPatternParser.h"
#include "DataHolders/SourceInfo.h"
#include <boost/filesystem.hpp>

using namespace std;
using json = nlohmann::json;
namespace fs = boost::filesystem;


json getApm(const string FilePath) {
  ifstream i(FilePath);
  json apm;
  i >> apm;
  return apm;
}

vector<std::shared_ptr<AstTracedVariable>> parseAstPattern(const json apm) {
  vector<std::shared_ptr<AstTracedVariable>> ret;

  for (int i = 0; i < apm.size(); ++i) {
    auto jData = apm[i];
    int index = jData["index"];
    string pattern = jData["pattern"];
    fs::path CmdDir(jData["commandDir"]);

    for (int j = 0; j < jData["ids"].size(); ++j) {
      int iid = j;
      auto id = jData["ids"][j];
      string traceType = id["tag"];

      if (pattern == "COND_ASSIGN" && !CondAssignVarTypes.count(traceType))
        continue;
      if (pattern == "COND_EXEC" && CondAssignVarTypes.count(traceType))
        continue;

      if (TracedVarTypes.count(traceType) || TracedVarBaseTypes.count(traceType)) {
        fs::path DecPath(id["file"]);
        fs::path RealDecPath;

        if (DecPath.is_relative())
          RealDecPath = fs::canonical(CmdDir / DecPath);
        else
          RealDecPath = fs::canonical(DecPath);

        int decLine = stoi((string) id["line"]);
        int decCol = stoi((string) id["col"]);

        std::shared_ptr<AstTracedVariable> p;
        p = make_shared<AstTracedVariable>(index, iid, pattern, traceType,
                                                    (SourceInfo) {RealDecPath.string(), decLine, decCol}, 
                                                    i);
        if (p->isField())
          p->setFieldOffset(id["offset"]);

        if (p->isCondExe()) {
          fs::path BodyPath(jData["stmt"]["body"]["start"]["file"]);
          fs::path RealBodyPath;

          if (BodyPath.is_relative())
            RealBodyPath = fs::canonical(CmdDir / BodyPath);
          else
            RealBodyPath = fs::canonical(BodyPath);

          p->setBodyStart((SourceInfo) {RealBodyPath.string(), 
                                stoi((string) jData["stmt"]["body"]["start"]["line"]), 
                                stoi((string) jData["stmt"]["body"]["start"]["col"])});
          p->setBodyEnd((SourceInfo) {RealBodyPath.string(), 
                                stoi((string) jData["stmt"]["body"]["end"]["line"]), 
                                stoi((string) jData["stmt"]["body"]["end"]["col"])});
        }

        ret.push_back(p);
      }
    }
  }
  return ret;
}

tracemap getDecToTraceMap(vector<std::shared_ptr<AstTracedVariable>> AstTracedVars) {
  tracemap DecMap;
  for (auto p : AstTracedVars) {
    string decPath = p->getDecSrcInfo().file;
    DecMap[decPath].insert(p);
  }
  return DecMap;
}
