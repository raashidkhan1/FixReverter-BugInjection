#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "json.hpp"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "GrammarLib/Grammar.h"
#include "GrammarLib/Validator.h"
using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using std::string;
using std::vector;
using json = nlohmann::json;

extern vector<json> jsons;
extern Grammar* grammar;
extern bool parseInfoOn;
template <typename Derived> class PatternRecursiveASTVisitor :
public RecursiveASTVisitor<Derived>
{
 protected:
  string fileName;
  string commandDir;
  std::ofstream* outFile;
  std::ofstream* statFile;
  unsigned int ifSize;
 public:
  void set(string f, string c, std::ofstream* o, std::ofstream* s, int i)
  {
    fileName = f;
    commandDir = c;
    outFile = o;
    statFile = s;
    ifSize = i;
  }
};


class PatternASTConsumer : public ASTConsumer
{
 protected: 
  template<typename Derived>
  void assign(PatternRecursiveASTVisitor<Derived>*v)
    {v->set(PAC_fileName,PAC_commandDir,PAC_outFile,PAC_statFile, PAC_ifSize);}
 public:
  static void set(string f, string c, std::ofstream* o, std::ofstream* s, int i)
  {
    PAC_fileName = f;
    PAC_commandDir = c;
    PAC_outFile = o;
    PAC_statFile = s;
    PAC_ifSize = i;
  }
  
  static string PAC_fileName;
  static string PAC_commandDir;
  static std::ofstream* PAC_outFile;
  static std::ofstream* PAC_statFile;
  static int PAC_ifSize;
};

class PatternASTFrontendAction : public ASTFrontendAction
{
 public:
  static string PAFA_fileName;
  static string PAFA_commandDir;
  static std::ofstream* PAFA_outFile;
  static std::ofstream* PAFA_statFile;
  static int PAFA_ifSize;
  static void set(string f, string c, std::ofstream* o, std::ofstream* s, int i)
  {
    PAFA_fileName = f;
    PAFA_commandDir = c;
    PAFA_outFile = o;
    PAFA_statFile = s;
    PAFA_ifSize = i;
  }
};

