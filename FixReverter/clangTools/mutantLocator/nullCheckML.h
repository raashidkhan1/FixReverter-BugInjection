#pragma once
#include "Util.h"
#include "utils.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class NullCheckMLVisitor
  : public RecursiveASTVisitor<NullCheckMLVisitor>
{
private:
  enum CondType {TRACE, UNTRACE, ALL};
  ASTContext *Context;
  string getConds(json, CondType);
  string getVar(vector<json> js, int x);
  string getInsertString(string endLoc,int mod=0);
public:
  static vector<string> injectLocs_Cond;
  static vector<json> vars_Cond;
  static vector<int> injectLocID_Cond;
  static vector<string> injectLocs_Stmt;
  static vector<string> endLocs_Stmt;
  static vector<string> injections_Stmt;
  static vector<int> injectLocID_Stmt;
  
  explicit NullCheckMLVisitor(ASTContext *context)
    : Context(context){}
      
  bool VisitIfStmt(IfStmt *stmt);
  bool VisitForStmt(ForStmt *stmt);
  bool VisitWhileStmt(WhileStmt *stmt);
//  bool VisitVarDecl(VarDecl *v);
  bool VisitDeclStmt(DeclStmt *v);
  bool VisitValueStmt(ValueStmt *v);
};

class NullCheckMLConsumer : public ASTConsumer
{
private:
  NullCheckMLVisitor visitor;

public:
  explicit NullCheckMLConsumer(ASTContext *context)
    : visitor(context){}
  
  virtual void HandleTranslationUnit(ASTContext &context) override
  {
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
};

class NullCheckMLAction : public ASTFrontendAction
{
public:
  virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &compiler, StringRef inFile) override
  {
    return std::unique_ptr<ASTConsumer>(new NullCheckMLConsumer(&compiler.getASTContext()));
  }
};
