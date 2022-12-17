#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
using namespace clang;
using namespace clang::tooling;
using std::string;
using std::vector;

#include "Pattern.h"
#include "tokenizer.h"
#include "utils.h"
#include "GrammarLib/Grammar.h"

static llvm::cl::OptionCategory APMCategory("ast pattern matcher tool options");
static llvm::cl::opt<string> outPathStr(
                                        "o",
                                        llvm::cl::desc("Path to output file"),
                                        llvm::cl::Required, llvm::cl::cat(APMCategory));
static llvm::cl::opt<string> statPathStr(
                                        "s",
                                        llvm::cl::desc("Path to file to write statistics on"),
                                        llvm::cl::Optional, llvm::cl::init("/dev/null"), llvm::cl::cat(APMCategory));
static llvm::cl::opt<string> grammarPathStr(
                                        "g",
                                        llvm::cl::desc("Path to grammar file to match ast patterns"),
                                        llvm::cl::Required, llvm::cl::cat(APMCategory));
static llvm::cl::opt<bool> parseInfo(
                                        "v",
                                        llvm::cl::desc("Verbosely prints information from building state chart, prints tokens found during runtime"),
                                        llvm::cl::Optional, llvm::cl::ValueDisallowed, llvm::cl::cat(APMCategory));

//global varialbes
string PatternASTConsumer::PAC_fileName;
string PatternASTConsumer::PAC_commandDir;
std::ofstream* PatternASTConsumer::PAC_outFile;
std::ofstream* PatternASTConsumer::PAC_statFile;
string PatternASTFrontendAction::PAFA_fileName;
string PatternASTFrontendAction::PAFA_commandDir;
std::ofstream* PatternASTFrontendAction::PAFA_outFile;
std::ofstream* PatternASTFrontendAction::PAFA_statFile;
int PatternASTConsumer::PAC_ifSize;
int PatternASTFrontendAction::PAFA_ifSize;

vector<json> jsons;
Grammar* grammar;
bool parseInfoOn;

int main(int argc, const char** argv) {
  CommonOptionsParser OptionsParser(argc, argv, APMCategory);
  CompilationDatabase &CompileDB = OptionsParser.getCompilations();
  const vector<string> &srcPathList = OptionsParser.getSourcePathList();

  //make sure input path is absolute
  getSrcPath(srcPathList);

  ClangTool Tool(CompileDB, srcPathList);

  std::ofstream outFile;
  std::ofstream statFile;

  outFile.open(outPathStr, std::ios::app);
  statFile.open(statPathStr, std::ios::app);

  pair<string, string> commandDirFilePair = getCommandDirFileStrPair(CompileDB, srcPathList);
  const string commandDir = commandDirFilePair.first;
  const string srcPath = commandDirFilePair.second;

  std::cout << srcPath << std::endl;
  PatternASTFrontendAction::set(srcPath, commandDir, &outFile, &statFile, 3);
  PatternASTConsumer::set(srcPath, commandDir, &outFile, &statFile, 3);
  vector<std::unique_ptr<FrontendActionFactory> > patterns;

  grammar = new Grammar(grammarPathStr);
  parseInfoOn = parseInfo;
  if (parseInfo)
    {
      std::cout << grammar->toString() << std::endl;
    }

  if (Tool.run(newFrontendActionFactory<TokenizerAction>().get()) != 0)
    std::cout << "Error tool failed\n";

  for (unsigned int i = 0; i < jsons.size(); ++i)
    {
      outFile << jsons[i] << std::endl;
    }
  outFile.close();
  return 0;
}
