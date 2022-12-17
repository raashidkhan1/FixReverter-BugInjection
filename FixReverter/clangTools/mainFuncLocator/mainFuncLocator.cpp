#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/filesystem.hpp>
#include "json.hpp"

#include "utils.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::driver;
using namespace std;
using json = nlohmann::json;
namespace fs = boost::filesystem;

//global variables
bool mainStmt = false;
fs::path srcPath;
fs::path commandDir;
vector<json> operations;

// Prototypes
class MainFuncInjectVistor;
class MainFuncLocateConsumer;
class MainFuncLocateAction;

static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::OptionCategory DICategory("delete-if tool options");
static llvm::cl::opt<string> outPathStr(
                                        "o",
                                        llvm::cl::desc("Path to output file"),
                                        llvm::cl::Required, llvm::cl::cat(DICategory));
static llvm::cl::opt<string> mainFuncName(
                                        "n",
                                        llvm::cl::desc("Name of main function, defaulted as main"),
                                        llvm::cl::Optional, llvm::cl::init("main"), llvm::cl::cat(DICategory));


class MainFuncInjectVistor
  : public RecursiveASTVisitor<MainFuncInjectVistor> {
private:
  ASTContext *Context;

public:
  explicit MainFuncInjectVistor(ASTContext *Context)
    : Context(Context) {}

  bool TraverseStmt (Stmt *S) {
    if (mainStmt) {    //if we are traversing the main function
      string beginLoc = S->getBeginLoc().printToString(Context->getSourceManager());
      vector<string> elems = stringSplit(beginLoc, ':');

      fs::path sourceFilePath(elems[0]);
      fs::path absSourceFilePath = sourceFilePath;
      if (sourceFilePath.is_relative())    //if compiliation database exists make sourceFilePath absolute
        absSourceFilePath = commandDir / sourceFilePath;

      if (fs::equivalent(absSourceFilePath, srcPath)) {
        json op;
        op["operator"] = "insert";
        op["index"] = -1;

        op["file"] = elems[0];
        op["start"]["line"] = stoi(elems[1]);
        op["start"]["col"] = stoi(elems[2]);
        op["content"] = "@FixReverterMainReplacement@";        

        operations.push_back(op);

        llvm::outs() << "[INFO] - successfully located first line of main function: " << beginLoc << "\n";

        return false;    //stop traversing after reaching first statement
      } else {
        llvm::errs() << "[ERROR] - something went wrong, failed to catch main function in file" << "\n";
        exit(0);
      }
    }
    return true;
  }

  bool TraverseFunctionDecl(FunctionDecl *fd) {
    string funcName = fd->getNameAsString();
    if (funcName == mainFuncName) {
      if (fd->doesThisDeclarationHaveABody()) {
        if (fd->hasBody()) {
          llvm::outs() << "[INFO] - reached main function" << "\n";

          vector<string> elems = stringSplit(fd->getBeginLoc().printToString(Context->getSourceManager()), ':');

          json op;
          op["operator"] = "insert";
          op["index"] = -1;

          op["file"] = elems[0];
          op["start"]["line"] = stoi(elems[1]);
          op["content"] = "@FixReverterMacroReplacement@";        

          if (fd->isExternC()) {
            llvm::outs() << "[INFO] - this function is extern C" << "\n";
            op["start"]["col"] = 1;
          } else
            op["start"]["col"] = stoi(elems[2]);

          operations.push_back(op);

          mainStmt = true;
          RecursiveASTVisitor<MainFuncInjectVistor>::TraverseStmt(fd->getBody());
          return false;
        }
      }
    }
    return true;
  }
};

class MainFuncLocateConsumer : public clang::ASTConsumer {
private:
  MainFuncInjectVistor Visitor;

public:
  explicit MainFuncLocateConsumer(ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class MainFuncLocateAction : public ASTFrontendAction {
public:
  virtual unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler,
                                                         StringRef file) {
    return unique_ptr<ASTConsumer>(new MainFuncLocateConsumer(&Compiler.getASTContext()));
  }
};

 
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, DICategory);
  CompilationDatabase &CompileDB = OptionsParser.getCompilations();

  const vector<string> &srcPathList = OptionsParser.getSourcePathList();
  srcPath = getSrcPath(srcPathList);

  pair<fs::path, fs::path> commandDirFilePair = getCommandDirFilePair(CompileDB, srcPathList);
  commandDir = commandDirFilePair.first;

  llvm::outs() << "[INFO] - start running..." << "\n";
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  Tool.run(newFrontendActionFactory<MainFuncLocateAction>().get());

  std::ofstream outFile;
  outFile.open(outPathStr, std::ios::app);
  outFile << std::setw(4) << operations;
  outFile.close();

}


