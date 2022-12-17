#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Pattern.h"
#include "utils.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using std::string;
using std::vector;

class TokenizerVisitor
  : public PatternRecursiveASTVisitor<TokenizerVisitor>
{
private:  
  ASTContext *Context;
  string CommandDir;
  string FileName;
  bool isConstVar(Stmt* stmt);
  bool isAsn(Stmt* stmt);
  bool isInteger(ValueDecl* vd);
  bool isPtr(ValueDecl* vd);
  bool isFunction(ValueDecl* vd);
  bool isDeclRefExpr(Stmt* stmt, ValueDecl*& var);
  bool inValidContext;
  bool isPrefixUnaryOperator(UnaryOperator* u);
  void processType(const clang::Type* t,string s, SourceLocation ls);
  Validator* validator;
  int count;
  string deref;
public:
  explicit TokenizerVisitor(ASTContext *context)
    : Context(context) {
    validator = new Validator(parseInfoOn);
    inValidContext = false;
    count=0;
  }
  
  bool TraverseIfStmt(IfStmt *stmt);
  bool TraverseForStmt(ForStmt *stmt);
  bool TraverseWhileStmt(WhileStmt *stmt);
  bool TraverseFunctionDecl(FunctionDecl *fd);
  bool VisitBinaryOperator(BinaryOperator* bo);
  bool VisitDeclRefExpr(DeclRefExpr* decl);
  bool VisitIntegerLiteral(IntegerLiteral* i);
  bool VisitFloatingLiteral(FloatingLiteral* i);
  bool TraverseCompoundStmt(CompoundStmt* c);
  bool TraverseReturnStmt(ReturnStmt* r);
  bool VisitGotoStmt(GotoStmt* g);
  bool VisitBreakStmt(BreakStmt* b);
  bool VisitContinueStmt(ContinueStmt* r);
  bool VisitUnaryOperator(UnaryOperator* u);
  bool VisitMemberExpr(MemberExpr* m);
  bool VisitArraySubscriptExpr(ArraySubscriptExpr* a);
  bool VisitConditionalOperator(ConditionalOperator* co);
  bool VisitCStyleCastExpr(CStyleCastExpr* c);
  bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* e);
  bool VisitCallExpr(CallExpr* c);
  bool VisitVarDecl(VarDecl *v);
  bool shortReturnCompound(Stmt* stmt);
  bool isEscape(Stmt* stmt);
  bool VisitValueStmt(ValueStmt *v);
};

class TokenizerConsumer : public PatternASTConsumer
{
private:
  TokenizerVisitor visitor;
public:
  explicit TokenizerConsumer(ASTContext *context)
    : visitor(context){}
  virtual void HandleTranslationUnit(ASTContext &context) override
  {
    assign(&visitor);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
};

class TokenizerAction : public PatternASTFrontendAction
{
public:
  virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &compiler,StringRef inFile) override
  {
  return std::unique_ptr<ASTConsumer>(new TokenizerConsumer(&compiler.getASTContext()));}
};
