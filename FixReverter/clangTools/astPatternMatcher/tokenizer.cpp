#include "tokenizer.h"

bool TokenizerVisitor::isDeclRefExpr(Stmt* stmt, ValueDecl*& var)
{
  if (ImplicitCastExpr::classof(stmt))
    return isDeclRefExpr(((ImplicitCastExpr*)stmt)->getSubExpr(),var);
  else if ( DeclRefExpr::classof(stmt))
    {
      var = ((DeclRefExpr*)stmt)->getDecl();
      return true;
    }
  return false;
}

bool TokenizerVisitor::shortReturnCompound(Stmt* stmt)
{
  if (!CompoundStmt::classof(stmt))
    return false;
  CompoundStmt* Cstmt = (CompoundStmt*)stmt;
  if (Cstmt->body_back() == nullptr)
    return false;
  return (Cstmt->size() <= ifSize) && (isEscape(Cstmt->body_back()));
}

bool TokenizerVisitor::isEscape(Stmt* stmt)
{
  return ReturnStmt::classof(stmt) ||
    BreakStmt::classof(stmt) ||
    GotoStmt::classof(stmt) ||
    ContinueStmt::classof(stmt);
}


void TokenizerVisitor::processType( const clang::Type* t,string s, SourceLocation ls)
{
  int ptrcount = 0;
  vector<const clang::ArrayType*> v;
  const clang::Type * onion = t;

  while (onion->isPointerType() || onion->isArrayType()) {
    if (onion->isPointerType()) {
      ptrcount++;
    } else {
      v.push_back(onion->getAsArrayTypeUnsafe());
    }
    onion = onion->getPointeeOrArrayElementType();
  }

  json jt;
  jt["val"] = string(onion->getTypeClassName());
  validator->processToken("t_type",jt,count++);

  json js;
  js["val"] = "*";
  for (int i = 0; i < ptrcount; ++i)
    validator->processToken("t_mult",js,count++);


  json jv;
  jv["val"] = s;
  validator->processToken("t_var",jv,count++);

  for (unsigned long i = 0; i < v.size(); ++i) {
    json jl;
    jl["val"] = "[";
    validator->processToken("t_brackL",jl,count++);
    if (ConstantArrayType::classof(v[i])) {
      json x;
      int va = ((const ConstantArrayType*)v[i])->getSize().getLimitedValue();
      x["val"] = std::to_string(va);
      // 0 matches with t_null and other integer values match with t_litNum
      if (va == 0)
        validator->processToken("t_null",x,count++);
      else
        validator->processToken("t_litNum",x,count++);

    } else if (DependentSizedArrayType::classof(v[i])) {
        RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(((const DependentSizedArrayType*)v[i])->getSizeExpr());
    } else if (VariableArrayType::classof(v[i])) {
        RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(((const VariableArrayType*)v[i])->getSizeExpr());
    }
    json jr;
    jr["val"] = "]";
    string varLoc = ls.printToString(Context->getSourceManager());
    const vector<string> elems = stringSplit(varLoc, ':');
    jr["file"] = elems[0];
    jr["line"] = elems[1];
    jr["col"] = elems[2];

    validator->processToken("t_brackR",jr,count++);

  }


}

bool TokenizerVisitor::TraverseIfStmt(IfStmt *stmt)
{
  // Create a new validator instance for each spot where a pattern is to be matched
  // saving current validator state in the case of an if statement inside the body of another if statement
  Validator* tempV = validator;
  bool tempC = inValidContext;
  inValidContext = true;
  int tempCount = count;
  count = 0;
  validator = new Validator(grammar,parseInfoOn);

  json jif;
  jif["val"] = "if";
  validator->processToken("t_if",jif,count++);

  Expr* cond = stmt->getCond();
  RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(cond);

  Stmt* then = stmt->getThen();
  if (!CompoundStmt::classof(then) && isAsn(then)){
    RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(((BinaryOperator*)then)->getLHS());
    json jx;
    jx["val"] = ";";
    validator->processToken("t_asnStmt", jx,count++);
  } else if (!CompoundStmt::classof(then) && !isEscape(then)) {
    json jx;
    jx["val"] = ";";
    validator->processToken("t_noJump", jx,count++);
    inValidContext = false;
    RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(then);
    inValidContext = true;
  } else {
    RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(then);
  }
  if (stmt->hasElseStorage())
    {
      json j;
      j["val"] = "else";
      validator->processToken("t_else", j,count++);
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(stmt->getElse());
    }

  json jfi;
  jfi["val"] = "fi";
  validator->processToken("t_fi",jfi,count++);

  string pat = validator->getPattern();
  if (pat != "")
    {
      SourceManager &sourceManager = Context->getSourceManager();
      json x;
      string beginLoc = sourceManager.getExpansionRange(stmt->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> stmtStart = stringSplit(beginLoc, ':');
      x["fileName"] = fileName;
      x["commandDir"] = commandDir;
      x["stmt"]["type"] = "if";
      x["stmt"]["start"]["file"] = stmtStart[0];
      x["stmt"]["start"]["line"] = stmtStart[1];
      x["stmt"]["start"]["col"] = stmtStart[2];

      string endLoc = sourceManager.getExpansionRange(stmt->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> stmtEnd = stringSplit(endLoc, ':');
      x["stmt"]["end"]["file"] = stmtEnd[0];
      x["stmt"]["end"]["line"] = stmtEnd[1];
      x["stmt"]["end"]["col"] = stmtEnd[2];


      string bodyStartLoc = sourceManager.getExpansionRange(then->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> bodyStart = stringSplit(bodyStartLoc, ':');
      x["stmt"]["body"]["start"]["file"] = bodyStart[0];
      x["stmt"]["body"]["start"]["line"] = bodyStart[1];
      x["stmt"]["body"]["start"]["col"] = bodyStart[2];

      string bodyEndLoc = sourceManager.getExpansionRange(then->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> bodyEnd = stringSplit(bodyEndLoc, ':');
      x["stmt"]["body"]["end"]["file"] = bodyEnd[0];
      x["stmt"]["body"]["end"]["line"] = bodyEnd[1];
      x["stmt"]["body"]["end"]["col"] = bodyEnd[2];

      x["pattern"] = pat;
      x["ids"] = validator->getJsons();
      x["tokenOrder"] = validator->getTokenOrder();
      jsons.push_back(x);
    }

  // restore previous validator instance
  validator = tempV;
  inValidContext = tempC;
  count = tempCount;
  return true;
}

bool TokenizerVisitor::VisitValueStmt(ValueStmt *v) {
  if (!inValidContext)
    {
      Validator* temp = validator;
      bool tempC = inValidContext;
      int tempCount = count;
      count = 0;
      inValidContext = true;
      validator = new Validator(grammar,parseInfoOn);

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(v->getExprStmt());
      json j;
      j["val"] = "semi";
      validator->processToken("t_semi",j,count++);
      string pat = validator->getPattern();
      if (pat != "")
	{
      SourceManager &sourceManager = Context->getSourceManager();
	  json x;
      string beginLoc = sourceManager.getExpansionRange(v->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> stmtStart = stringSplit(beginLoc, ':');
      x["fileName"] = fileName;
      x["commandDir"] = commandDir;
	  x["stmt"]["type"] = "value";
      x["stmt"]["start"]["file"] = stmtStart[0];
      x["stmt"]["start"]["line"] = stmtStart[1];
      x["stmt"]["start"]["col"] = stmtStart[2];

      string endLoc = sourceManager.getExpansionRange(v->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> stmtEnd = stringSplit(endLoc, ':');
      x["stmt"]["end"]["file"] = stmtEnd[0];
      x["stmt"]["end"]["line"] = stmtEnd[1];
      x["stmt"]["end"]["col"] = stmtEnd[2];

	  x["pattern"] = pat;
	  x["ids"] = validator->getJsons();
	  x["tokenOrder"] = validator->getTokenOrder();
	  jsons.push_back(x);
	}

      validator = temp;
      inValidContext = tempC;
      count = tempCount;
      return true;
    }
  return true;
}

bool TokenizerVisitor::VisitVarDecl(VarDecl *v) {
  if (!inValidContext)
    {
      Validator* temp = validator;
      bool tempC = inValidContext;
      int tempCount = count;
      count = 0;
      inValidContext = true;
      validator = new Validator(grammar,parseInfoOn);
      processType(v->getType().getTypePtr(),v->getNameAsString(), v->getTypeSpecEndLoc());

      json j_s;
      j_s["val"] = "semi";
      validator->processToken("t_semi",j_s,count++);
      string pat = validator->getPattern();
      if (pat != "")
	{
      SourceManager &sourceManager = Context->getSourceManager();
	  json x;
      string beginLoc = sourceManager.getExpansionRange(v->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> stmtStart = stringSplit(beginLoc, ':');
      x["fileName"] = fileName;
      x["commandDir"] = commandDir;
	  x["stmt"]["type"] = "dec";
      x["stmt"]["start"]["file"] = stmtStart[0];
      x["stmt"]["start"]["line"] = stmtStart[1];
      x["stmt"]["start"]["col"] = stmtStart[2];

      string endLoc = sourceManager.getExpansionRange(v->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> stmtEnd = stringSplit(endLoc, ':');
      x["stmt"]["end"]["file"] = stmtEnd[0];
      x["stmt"]["end"]["line"] = stmtEnd[1];
      x["stmt"]["end"]["col"] = stmtEnd[2];

	  x["pattern"] = pat;
	  x["ids"] = validator->getJsons();
	  x["tokenOrder"] = validator->getTokenOrder();
	  jsons.push_back(x);
	}

      validator = temp;
      inValidContext = tempC;
      count = tempCount;
      //return true;
    }
  return true;
}

bool TokenizerVisitor::TraverseForStmt(ForStmt *stmt) {
  Validator* tempV = validator;
  bool tempC = inValidContext;
  inValidContext = true;
  int tempCount = count;
  count = 0;
  validator = new Validator(grammar,parseInfoOn);

  json jif;
  jif["val"] = "for";
  validator->processToken("t_for",jif,count++);



  Expr* cond = stmt->getCond();
  RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(cond);

  Stmt* then = stmt->getBody();
  if (!CompoundStmt::classof(then) && !isEscape(then)) {
    json jx;
    jx["val"] = ";";
    validator->processToken("t_noJump", jx,count++);
  }
  RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(then);


  json jfi;
  jfi["val"] = "rof";
  validator->processToken("t_rof",jfi,count++);


  string pat = validator->getPattern();
  if (pat != "")
    {
      SourceManager &sourceManager = Context->getSourceManager();
      json x;
      string beginLoc = sourceManager.getExpansionRange(stmt->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> stmtStart = stringSplit(beginLoc, ':');
      x["fileName"] = fileName;
      x["commandDir"] = commandDir;
	  x["stmt"]["type"] = "for";
      x["stmt"]["start"]["file"] = stmtStart[0];
      x["stmt"]["start"]["line"] = stmtStart[1];
      x["stmt"]["start"]["col"] = stmtStart[2];

      string endLoc = sourceManager.getExpansionRange(stmt->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> stmtEnd = stringSplit(endLoc, ':');
      x["stmt"]["end"]["file"] = stmtEnd[0];
      x["stmt"]["end"]["line"] = stmtEnd[1];
      x["stmt"]["end"]["col"] = stmtEnd[2];

      string bodyStartLoc = sourceManager.getExpansionRange(then->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> bodyStart = stringSplit(bodyStartLoc, ':');
      x["stmt"]["body"]["start"]["file"] = bodyStart[0];
      x["stmt"]["body"]["start"]["line"] = bodyStart[1];
      x["stmt"]["body"]["start"]["col"] = bodyStart[2];

      string bodyEndLoc = sourceManager.getExpansionRange(then->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> bodyEnd = stringSplit(bodyEndLoc, ':');
      x["stmt"]["body"]["end"]["file"] = bodyEnd[0];
      x["stmt"]["body"]["end"]["line"] = bodyEnd[1];
      x["stmt"]["body"]["end"]["col"] = bodyEnd[2];

      x["pattern"] = pat;
      x["ids"] = validator->getJsons();
      x["tokenOrder"] = validator->getTokenOrder();
      jsons.push_back(x);
    }


  // restore previous validator instance
  validator = tempV;
  inValidContext = tempC;
  count = tempCount;
  return true;

}

bool TokenizerVisitor::TraverseWhileStmt(WhileStmt *stmt) {
  Validator* tempV = validator;
  bool tempC = inValidContext;
  inValidContext = true;
  int tempCount = count;
  count = 0;
  validator = new Validator(grammar,parseInfoOn);

  json jif;
  jif["val"] = "while";
  validator->processToken("t_while",jif,count++);

  Expr* cond = stmt->getCond();
  RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(cond);

  Stmt* then = stmt->getBody();
  if (!CompoundStmt::classof(then) && !isEscape(then)) {
    json jx;
    jx["val"] = ";";
    validator->processToken("t_noJump", jx,count++);
  }
  RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(then);


  json jfi;
  jfi["val"] = "elihw";
  validator->processToken("t_elihw",jfi,count++);


  string pat = validator->getPattern();
  if (pat != "")
    {
 SourceManager &sourceManager = Context->getSourceManager();
      json x;
      string beginLoc = sourceManager.getExpansionRange(stmt->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> stmtStart = stringSplit(beginLoc, ':');
      x["fileName"] = fileName;
      x["commandDir"] = commandDir;
	  x["stmt"]["type"] = "while";
      x["stmt"]["start"]["file"] = stmtStart[0];
      x["stmt"]["start"]["line"] = stmtStart[1];
      x["stmt"]["start"]["col"] = stmtStart[2];

      string endLoc = sourceManager.getExpansionRange(stmt->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> stmtEnd = stringSplit(endLoc, ':');
      x["stmt"]["end"]["file"] = stmtEnd[0];
      x["stmt"]["end"]["line"] = stmtEnd[1];
      x["stmt"]["end"]["col"] = stmtEnd[2];

      string bodyStartLoc = sourceManager.getExpansionRange(then->getBeginLoc()).getBegin().printToString(sourceManager);
      const vector<string> bodyStart = stringSplit(bodyStartLoc, ':');
      x["stmt"]["body"]["start"]["file"] = bodyStart[0];
      x["stmt"]["body"]["start"]["line"] = bodyStart[1];
      x["stmt"]["body"]["start"]["col"] = bodyStart[2];

      string bodyEndLoc = sourceManager.getExpansionRange(then->getEndLoc()).getEnd().printToString(sourceManager);
      const vector<string> bodyEnd = stringSplit(bodyEndLoc, ':');
      x["stmt"]["body"]["end"]["file"] = bodyEnd[0];
      x["stmt"]["body"]["end"]["line"] = bodyEnd[1];
      x["stmt"]["body"]["end"]["col"] = bodyEnd[2];

      x["pattern"] = pat;
      x["ids"] = validator->getJsons();
      x["tokenOrder"] = validator->getTokenOrder();
      jsons.push_back(x);
    }



  // restore previous validator instance
  validator = tempV;
  inValidContext = tempC;
  count = tempCount;
  return true;

}

bool TokenizerVisitor::TraverseFunctionDecl(FunctionDecl *fd)
{
  unsigned int numJ = jsons.size();
  SourceLocation loc = fd->getInnerLocStart();
  //std::cout << ((Context->getSourceManager()).getFilename(loc)).str() << "->" << fd->getNameAsString() << std::endl;
  if ((((Context->getSourceManager()).getFilename(loc)).str() == fileName) &&
      fd->doesThisDeclarationHaveABody())
    {
      if (fd->hasBody())
        {
          RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(fd->getBody());
          if (jsons.size() > numJ)
            {
              for (unsigned int i = numJ; i < jsons.size(); ++i)
                {
                  jsons[i]["func"]["name"] = fd->getNameAsString();
                  for (unsigned int j = 0; j < fd->param_size(); ++j)
                    {
                      TypeInfo t = fd->getParamDecl(j)->getASTContext().getTypeInfo(fd->getParamDecl(j)->getType());
                      uint64_t typeSize = t.Width;
                      jsons[i]["func"]["param"][j] = typeSize;
                    }
                }
            }
        }
    }
  return true;
}

bool TokenizerVisitor::isConstVar(Stmt* stmt)
{
  if (ImplicitCastExpr::classof(stmt))
    return isConstVar(((ImplicitCastExpr*)stmt)->getSubExpr());
  ValueDecl* val = nullptr;
  if (isDeclRefExpr(stmt, val))
    {
      QualType q = val->getType();
      return q.isConstQualified();
    }
  return false;
}

bool TokenizerVisitor::isAsn(Stmt* stmt)
{
  if (!BinaryOperator::classof(stmt))
    return false;
  return ((BinaryOperator*)stmt)->getOpcodeStr() == "=";
}

bool TokenizerVisitor::isInteger(ValueDecl* vd)
{
  const clang::Type* t = vd->getType().getTypePtr();
  if (const auto *BT = dyn_cast<BuiltinType>(t->getCanonicalTypeInternal()))
    return ((BT->getKind() >= BuiltinType::UShort && BT->getKind() <= BuiltinType::UInt128) ||
            (BT->getKind() >= BuiltinType::Short && BT->getKind() <= BuiltinType::Int128));
  return false;
}

bool TokenizerVisitor::isPtr(ValueDecl* vd)
{
  const clang::Type* t = vd->getType().getTypePtr();
  return t->isPointerType() || t->isArrayType();
}


bool TokenizerVisitor::isFunction(ValueDecl* vd)
{
  return vd->isFunctionOrFunctionTemplate();
}

bool TokenizerVisitor::VisitIntegerLiteral(IntegerLiteral* i)
{
  if (inValidContext)
    {
      int64_t v = i->getValue().getSExtValue();
      json x;
      x["val"] = std::to_string(v);
      // 0 matches with t_null and other integer values match with t_litNum
      if (v == 0)
        validator->processToken("t_null",x,count++);
      else
        validator->processToken("t_litNum",x,count++);
    }
  return true;
}

bool TokenizerVisitor::VisitFloatingLiteral(FloatingLiteral* i)
{
  if (inValidContext)
    {
      // throws exception if value is not IEEEdouble or shorter semantics
//      double v = i->getValue().convertToDouble();
      double v = i->getValueAsApproximateDouble();
      json x;
      x["val"] = std::to_string(v);
      validator->processToken("t_litFloat",x,count++);
    }
  return true;
}

bool TokenizerVisitor::VisitDeclRefExpr(DeclRefExpr* decl)
{
  if (inValidContext)
    {
      if (isConstVar(decl))
        {
          ValueDecl* v = decl->getDecl();
          if (v != NULL)
            {
              string varLoc = v->getLocation().printToString(Context->getSourceManager());
              json x;
              x["val"] = v->getNameAsString();
              const vector<string> elems = stringSplit(varLoc, ':');
              x["file"] = elems[0];
              x["line"] = elems[1];
              x["col"] = elems[2];
              validator->processToken("t_const", x,count++);

            }
        }
      else
        {

          ValueDecl* v = decl->getDecl();
          if (v != NULL)
            {
              string varLoc = v->getLocation().printToString(Context->getSourceManager());
              json x;
              x["val"] = v->getNameAsString();
              const vector<string> elems = stringSplit(varLoc, ':');
              x["file"] = elems[0];
              x["line"] = elems[1];
              x["col"] = elems[2];
              if (isPtr(v))
                validator->processToken("t_ptrVar",x,count++);
              else if (isInteger(v))
                validator->processToken("t_num",x,count++);
              else if (isFunction(v))
                validator->processToken("t_function",x,count++);
              else
                validator->processToken("t_var",x,count++);
              return true;
            }
        }
    }
  return true;
}

bool TokenizerVisitor::VisitBinaryOperator(BinaryOperator* bo)
{
  if (inValidContext)
    {

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(bo->getLHS());
      json x;
      x["val"] = bo->getOpcodeStr();
      if(bo->getOpcodeStr() == "==")
        validator->processToken("t_eqEq",x,count++);
      else if(bo->getOpcodeStr() == "!=")
        validator->processToken("t_nEq",x,count++);
      else if(bo->getOpcodeStr() == "&&")
        validator->processToken("t_and",x,count++);
      else if(bo->getOpcodeStr() == "||")
        validator->processToken("t_or",x,count++);
      else if(bo->getOpcodeStr() == "<")
        validator->processToken("t_les",x,count++);
      else if(bo->getOpcodeStr() == ">")
        validator->processToken("t_gtr",x,count++);
      else if(bo->getOpcodeStr() == "<=")
        validator->processToken("t_lesEq",x,count++);
      else if(bo->getOpcodeStr() == ">=")
        validator->processToken("t_gtrEq",x,count++);
      else if(bo->getOpcodeStr() == "+")
        validator->processToken("t_plus",x,count++);
      else if(bo->getOpcodeStr() == "-")
        validator->processToken("t_minus",x,count++);
      else if(bo->getOpcodeStr() == "*")
        validator->processToken("t_mult",x,count++);
      else if(bo->getOpcodeStr() == "/")
        validator->processToken("t_div",x,count++);
      else if(bo->getOpcodeStr() == "%")
        validator->processToken("t_mod",x,count++);
      else if(bo->getOpcodeStr() == "<<")
        validator->processToken("t_shiftL",x,count++);
      else if(bo->getOpcodeStr() == ">>")
        validator->processToken("t_shiftR",x,count++);
      else if(bo->getOpcodeStr() == "&")
        validator->processToken("t_bitAnd",x,count++);
      else if(bo->getOpcodeStr() == "^")
        validator->processToken("t_bitXor",x,count++);
      else if(bo->getOpcodeStr() == "|")
        validator->processToken("t_bitOr",x,count++);
      else if(bo->getOpcodeStr() == "=")
        validator->processToken("t_assign",x,count++);

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(bo->getRHS());

      return false;
    }
  return true;
}

bool TokenizerVisitor::TraverseCompoundStmt(CompoundStmt* c)
{
  if (inValidContext)
    {

      auto iter = c->body_begin();
      Validator* tempV = validator;
      int tempCount = count;

      json l;
      l["val"] = "less3";
      validator = new Validator(grammar,parseInfoOn);
      inValidContext = false;
      for(; iter != c->body_end(); std::advance(iter, 1))
	RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(*iter);
      inValidContext = true;
      // if we have not reached the end of the compound statement,
      // process the rest of the compound body
      delete validator;
      validator = tempV;
      count = tempCount;

      if (shortReturnCompound(c)){
	  validator->processToken("t_less3",l,count++);
      } else if (c->body_back() != nullptr && !isEscape(c->body_back())) {
	l["val"] = ";";
	validator->processToken("t_noJump",l,count++);
      }
    }
  else
    {
      for(auto i : c->body())
        RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(i);
    }
  return true;
}

bool TokenizerVisitor::TraverseReturnStmt(ReturnStmt* r)
{
  if (inValidContext)
    {
      json x;
      x["val"] = "return";
      inValidContext = false;
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(r->getRetValue());
      validator->processToken("t_return",x,count++);
      inValidContext = true;
    }
  else
    {
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(r->getRetValue());
    }
  return true;
}

bool TokenizerVisitor::VisitGotoStmt(GotoStmt* g)
{
  json x;
  x["val"] = "goto";

  if (inValidContext)
    validator->processToken("t_goto",x,count++);
  return true;
}

bool TokenizerVisitor::VisitBreakStmt(BreakStmt* b)
{
  json x;
  x["val"] = "break";

  if (inValidContext)
    validator->processToken("t_break", x,count++);
  return true;
}

bool TokenizerVisitor::VisitContinueStmt(ContinueStmt* r)
{
  json x;
  x["val"] = "continue";
  if (inValidContext)
    validator->processToken("t_continue",x,count++);
  return true;
}

bool TokenizerVisitor::VisitUnaryOperator(UnaryOperator* u)
{
  if (inValidContext)
    {
      if (u->isPostfix())
        RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(u->getSubExpr());

      json x;
      x["val"] = UnaryOperator::getOpcodeStr(u->getOpcode()).str();
      if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "++")
        validator->processToken("t_inc",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "--")
        validator->processToken("t_dec",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "+")
        validator->processToken("t_pos",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "-")
        validator->processToken("t_neg",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "!")
        validator->processToken("t_not",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "~")
        validator->processToken("t_flip",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "*")
        validator->processToken("t_deref",x,count++);
      else if(UnaryOperator::getOpcodeStr(u->getOpcode()).str() == "&")
        validator->processToken("t_ref",x,count++);

      if (u->isPrefix() || isPrefixUnaryOperator(u))
        RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(u->getSubExpr());
      return false;
    }
  return true;

}

bool TokenizerVisitor::isPrefixUnaryOperator(UnaryOperator* u) {
	string opcode = UnaryOperator::getOpcodeStr(u->getOpcode()).str();
	return opcode == "-" || opcode == "+" || opcode == "!" || opcode == "~" || opcode == "*" || opcode == "&";
}


bool TokenizerVisitor::VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr* e)
{
  if (inValidContext && !e->isArgumentType())
    {
      UnaryExprOrTypeTrait u = e->getKind();
      if (u == UnaryExprOrTypeTrait::UETT_SizeOf )
        {
          json x;
          x["val"] = "sizeof";
          validator->processToken("t_sizeof",x,count++);
          json z;
          z["val"] = "(";
          validator->processToken("t_parenL",z,count++);
          RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(e->getArgumentExpr());
          json y;
          y["val"] = ")";
          validator->processToken("t_parenR",y,count++);
          return false;
        }
    }
  return true;
}

bool TokenizerVisitor::VisitMemberExpr(MemberExpr* m)
{
  if (inValidContext)
    {
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(m->getBase());

      json x;
      if (m->isArrow())
        {
          x["val"] = "->";
          validator->processToken("t_derefFieldAcc",x,count++);
        }
      else
        {
          x["val"] = ".";
          validator->processToken("t_fieldAcc",x,count++);
        }

      ValueDecl* v = m->getMemberDecl();
      if (v != NULL)
        {
          string varLoc = v->getLocation().printToString(Context->getSourceManager());
          json y;
          y["val"] = v->getNameAsString();
          const vector<string> elems = stringSplit(varLoc, ':');
          y["file"] = elems[0];
          y["line"] = elems[1];
          y["col"] = elems[2];
          if (auto fd = dyn_cast_or_null<FieldDecl>(v))
            y["offset"] = fd->getFieldIndex();
          if (isPtr(v))
            validator->processToken("t_ptrVar",y,count++);
          else if (isInteger(v))
            validator->processToken("t_num",y,count++);
          else if (isFunction(v))
            validator->processToken("t_function",y,count++);
          else
            validator->processToken("t_var",y,count++);
        }
      return false;
    }
  return true;
}

bool TokenizerVisitor::VisitArraySubscriptExpr(ArraySubscriptExpr* a)
{
  if (inValidContext)
    {
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(a->getBase());

      json x;
      x["val"] = "[";
      validator->processToken("t_brackL",x,count++);

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(a->getIdx());

      json y;
      y["val"] = "]";
      validator->processToken("t_brackR",y,count++);
      return false;
    }
  return true;
}

bool TokenizerVisitor::VisitConditionalOperator(ConditionalOperator* co)
{
  if (inValidContext)
    {
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(co->getCond());

      json x;
      x["val"] = "?";
      validator->processToken("t_condThen",x,count++);

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(co->getTrueExpr());

      json y;
      y["val"] = ":";
      validator->processToken("t_condElse",y,count++);

      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(co->getFalseExpr());

      return false;
    }
  return true;
}

bool TokenizerVisitor::VisitCStyleCastExpr(CStyleCastExpr* c)
{
  if (inValidContext)
    {
      json x;
      x["val"] = "(" + c->getTypeAsWritten().getAsString() + ")";
      validator->processToken("t_cast",x,count++);
    }
  return true;
}

bool TokenizerVisitor::VisitCallExpr(CallExpr* c)
{
  if (inValidContext)
    {
      RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(c->getCallee());
      json z;
      z["val"] = "(";
      validator->processToken("t_parenL",z,count++);
      for (clang::CallExpr::arg_iterator a = c->arg_begin(); a < c->arg_end(); a++)
        {
          RecursiveASTVisitor<TokenizerVisitor>::TraverseStmt(*a);
          if (a + 1 < c->arg_end())
            {
              json comma;
              comma["val"] = ",";
              validator->processToken("t_comma",comma,count++);
            }
        }
      json y;
      y["val"] = ")";
      string varLoc = c->getRParenLoc().printToString(Context->getSourceManager());
      const vector<string> elems = stringSplit(varLoc, ':');
      y["file"] = elems[0];
      y["line"] = elems[1];
      y["col"] = elems[2];
      validator->processToken("t_parenR",y,count++);
      return false;
    }
  return true;
}
