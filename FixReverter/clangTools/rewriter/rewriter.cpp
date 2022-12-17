#include <memory>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
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
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTConsumer.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#include "Operation.h"
#include "utils.h"

#include "json.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace clang::driver;
using namespace llvm;
namespace fs = boost::filesystem;
using json = nlohmann::json;

// Prototypes
class EditorASTConsumer;
class EditorFrontendAction;

// Global variables
std::vector<Operation> operations; // Operations grouped by file
std::string srcPathStr;

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.

// A help message for this specific tool can be added afterwards.
// static cl::extrahelp MoreHelp("\nMore help text...\n");

// Command line arguments
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::OptionCategory RWCategory("inclusion locator tool options");
static llvm::cl::opt<std::string> inPathStr(
                                        "i",
                                        llvm::cl::desc("Path to input file"),
                                        llvm::cl::Required, llvm::cl::cat(RWCategory));

class EditorASTConsumer : public clang::ASTConsumer {
private:

public:
  explicit EditorASTConsumer(CompilerInstance *CI) {
  } 

  virtual void HandleTranslationUnit(ASTContext &context) override {
    SourceManager &sourceManager = context.getSourceManager();
    FileManager &fileManager = sourceManager.getFileManager();

    Rewriter rewriter;
    rewriter.setSourceMgr(sourceManager, context.getLangOpts());

    const FileEntry* file = fileManager.getFile(srcPathStr, true).get();
    FileID fileID =
      sourceManager.getOrCreateFileID(file, SrcMgr::C_User);
    std::cout << "Created FileID" << std::endl;

    if (file) {
      for (Operation op : operations) {
        std::cout << "Starting: " << op.toString() << std::endl;

        SourceLocation start =
          sourceManager.translateLineCol(fileID, op.line, op.col);
        std::cout << "Created start location" << std::endl;


        SourceLocation end;
        if (op.type == DELETE || op.type == REPLACE) {
          end = sourceManager.translateLineCol(fileID, op.lineEnd, op.colEnd);
          std::cout << "Created end location" << std::endl;
        }

        if (op.type == DELETE) {
          SourceRange range(start, end);

          // In reality, we want to do a replace
          // We want to maintain line numbers, so we use a replace to replace removed line breaks
          // The number of line breaks we want to add back in is the difference in start and end
          // line numbers, minus one
          int newLineBreaks = op.lineEnd - op.line;
          std::cout << "Created range" << std::endl;
          rewriter.ReplaceText(range, std::string(newLineBreaks, '\n'));
          std::cout << "Removed text in range" << std::endl;
        } else if (op.type == REPLACE) {
          SourceRange range(start, end);
          std::cout << "Created range" << std::endl;
          // Here, we could do
          // rewriter.ReplaceText(range, op.str);
          // but this wouldn't maintain indentation. So, instead we do this:
          rewriter.RemoveText(range);
          rewriter.InsertText(start, op.str, /* insertAfter */ true, /* indentNewLines */ true);
          std::cout << "Replaced text in range" << std::endl;
        } else if (op.type == INSERT) {
          rewriter.InsertText(start, op.str, /* insertAfter */ true, /* indentNewLines */ true);
        }
      }
    }  else {
        std::cerr << "ERROR: File " << srcPathStr << "not found." << std::endl;
        exit(0);
    }

    std::cout << "Rewriter operations complete. Beginning write to file" << std::endl;

    std::error_code EC;
    raw_fd_ostream ostream(srcPathStr, EC,
                              sys::fs::CreationDisposition::CD_CreateAlways);

    std::cout << "Created output stream." << std::endl;

    rewriter.getEditBuffer(fileID).write(ostream);

    if (EC) {
      std::cout << "ERROR in writing to file with error code " << EC
                << std::endl;
    } else {
      std::cout << "Wrote to file." << std::endl;
    }

    ostream.close();
    exit(0);  //exit here so that it will not be rewritten again
  }
};

class EditorFrontendAction : public ASTFrontendAction {
public:

  virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                         StringRef file) override {
    std::cout << "CreateAstConsumer" << std::endl;
    return std::make_unique<EditorASTConsumer>(&CI);
  }
};

json getApm(const std::string FilePath) {
  std::ifstream i(FilePath);
  json apm;
  i >> apm;
  return apm;
}

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, RWCategory);
  const std::vector<std::string> &srcPathList = OptionsParser.getSourcePathList();
  srcPathStr = getSrcPath(srcPathList).string();
  
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  auto jData = getApm(inPathStr);

  for (unsigned long i = 0; i < jData.size(); ++i) {
    auto j = jData[i];
    Operation operation;

    operation.index = j["index"];
    operation.fileName = j["file"];
    operation.line = j["start"]["line"];
    operation.col = j["start"]["col"];

    std::cout << j["operator"].get<std::string>() << std::endl;

    if (j["operator"].get<std::string>() == "insert")
      operation.type = INSERT;
    else if (j["operator"].get<std::string>() == "delete")
      operation.type = DELETE;
    else if (j["operator"].get<std::string>() == "replace")
      operation.type = REPLACE;
    else
      std::cout << "should not happen" << std::endl;

    std::cout << operation.type << std::endl;

    if (operation.type == DELETE || operation.type == REPLACE) {
      operation.lineEnd = j["end"]["line"];
      operation.colEnd = j["end"]["col"];
    }


    if (operation.type == INSERT || operation.type == REPLACE) {
      auto content = j["content"].get<std::string>();
      boost::replace_all(content, "@", "\n");
      operation.str = content;
    }

    operations.push_back(operation);
    std::cout << "Parsed command: "<< operation.toString() << std::endl;
  }

  std::cout << "Starting tool..." << std::endl;
  Tool.run(newFrontendActionFactory<EditorFrontendAction>().get());
}

