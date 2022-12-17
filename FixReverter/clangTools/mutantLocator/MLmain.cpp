#include "Util.h"
#include "nullCheckML.h"
#include "utils.h"

static cl::OptionCategory MLCategory("ML tool options");
// Command line arguments
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::OptionCategory DICategory("mutant locator tool options");
static llvm::cl::opt<string> outPathStr(
                                        "o",
                                        llvm::cl::desc("Path to output file"),
                                        llvm::cl::Required, llvm::cl::cat(DICategory));
static llvm::cl::opt<string> inPathStr(
                                        "i",
                                        llvm::cl::desc("Path to input file"),
                                        llvm::cl::Required, llvm::cl::cat(DICategory));

vector<json> operations = vector<json>();

void getCondMatches(json x, vector<string>& il, vector<int>& id, vector<json>& vn)
{
  json var = x;
  string str = "";

  if (x["stmt"]["type"] == "if" || x["stmt"]["type"] == "for" || x["stmt"]["type"] == "while") {
    std::ostringstream oss;
  
    oss << (string) x["stmt"]["start"]["file"] << ":" << (string) x["stmt"]["start"]["line"] << ":" << (string) x["stmt"]["start"]["col"];
    string locStr = oss.str();
  
    vn.push_back(var);
    il.push_back(locStr);
    id.push_back(x["index"]);
  }
  return;
}

void getStmtMatches(json x, vector<string>& il, vector<string>& el, vector<string>& is, vector<int>& ii)
{
  if (x["stmt"]["type"] == "dec" || x["stmt"]["type"] == "value") {
    std::ostringstream oss;
    oss << (string) x["stmt"]["start"]["file"] << ":" << (string) x["stmt"]["start"]["line"] << ":" << (string) x["stmt"]["start"]["col"];
    string locStr = oss.str();

    il.push_back(locStr);
    ii.push_back(x["index"]);
    if (x["pattern"] == "ARRAY_ENLARGE") {
      is.push_back("-1");
    } else if (x["pattern"] == "MEMCPY_MOVE") {
      is.push_back("+1");
    }else if (x["pattern"] == "MEMSET") {
      is.push_back("+1");
    }

    for (json j : x["ids"]) {
      if (j.find("tag") != j.end() && j["tag"] == "end") {
	std::ostringstream ossi;
	ossi << (string) j["file"] << ":" << (string) j["line"] << ":" << (string) j["col"];
	string lStr = ossi.str();
	el.push_back(lStr);
	break;
      }
    }
  }
}

vector<string> NullCheckMLVisitor::injectLocs_Cond = vector<string>();
vector<int> NullCheckMLVisitor::injectLocID_Cond = vector<int>();
vector<json> NullCheckMLVisitor::vars_Cond = vector<json>();

vector<string> NullCheckMLVisitor::injectLocs_Stmt = vector<string>();
vector<string> NullCheckMLVisitor::endLocs_Stmt = vector<string>();
vector<string> NullCheckMLVisitor::injections_Stmt = vector<string>();
vector<int> NullCheckMLVisitor::injectLocID_Stmt = vector<int>();


int main(int argc, const char** argv)
{
  CommonOptionsParser OptionsParser(argc, argv, MLCategory);

  const vector<string> &srcPathList = OptionsParser.getSourcePathList();
  getSrcPath(srcPathList);
  
  std::ifstream iF(inPathStr);
  json x;
  iF >> x;
  for (json j : x)
    {
      getCondMatches(j, NullCheckMLVisitor::injectLocs_Cond, NullCheckMLVisitor::injectLocID_Cond, NullCheckMLVisitor::vars_Cond);
      getStmtMatches(j, NullCheckMLVisitor::injectLocs_Stmt, NullCheckMLVisitor::endLocs_Stmt, NullCheckMLVisitor::injections_Stmt, NullCheckMLVisitor::injectLocID_Stmt);
    }

  ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  int result = Tool.run(newFrontendActionFactory<NullCheckMLAction>().get());

  std::ofstream outFile;
  outFile.open(outPathStr, std::ios::app);
  outFile << std::setw(4) << operations;
  outFile.close();
  return result;
}
