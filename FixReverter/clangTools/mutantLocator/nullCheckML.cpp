#include "nullCheckML.h"

string NullCheckMLVisitor::getInsertString(string endLoc, int mod) {
  const vector<string> elems = stringSplit(endLoc, ':');
  std::stringstream ss;
  //increment col by 1 for insertion
  ss << elems[0] << ":" << stoi(elems[1]) << ":" << stoi(elems[2]) + 1 + mod;
  return ss.str();
}

//bool NullCheckMLVisitor::VisitVarDecl(VarDecl *v) {
//  if (v->hasInit())
//    return true;
//  SourceManager &sourceManager = Context->getSourceManager();
//  string loc = v->getBeginLoc().printToString(sourceManager);
//  for (unsigned int i = 0; i < injectLocs_Stmt.size(); ++i) {
//    if (loc == injectLocs_Stmt[i]) {
//      clang::LangOptions lo;
//      string stmt_str;
//      llvm::raw_string_ostream outstream(stmt_str);
//
//      v->getParent()->printPretty(outstream, NULL, PrintingPolicy(lo));
//
//      SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(v->getEndLoc(), *Context);
//      SourceLocation endLoc;
//      if (semiLoc == SourceLocation())
//        endLoc = sourceManager.getExpansionRange(v->getEndLoc()).getEnd();
//      else
//        endLoc =semiLoc;
//
//      std::stringstream endStream;
//      endStream << "@#ifdef FRCOV@";
//      endStream << "}@";
//      endStream << "#endif";
//      operations.push_back(getInsertOp(injectLocID_Stmt[i], endLoc.printToString(sourceManager), endStream.str(), 1));
//
//      operations.push_back(getInsertOp(injectLocID_Stmt[i], endLocs_Stmt[i], injections_Stmt[i], 0));
//
//      std::stringstream injectStream;
//      injectStream << "@#ifdef FRCOV@";
//      injectStream << "if(!FIXREVERTER[" << injectLocID_Stmt[i] << "])@";
//      injectStream << "  " << stmt_str << ";@";
//      injectStream << "else {@";
//      injectStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Stmt[i] << "\\n\");@";
//      injectStream << "#endif@";
//      operations.push_back(getInsertOp(injectLocID_Stmt[i], loc, injectStream.str(), 0));
//    }
//  }
//  return true;
//}

bool NullCheckMLVisitor::VisitDeclStmt(DeclStmt *v) {
  if (!v->isSingleDecl())
    return true;

  SourceManager &sourceManager = Context->getSourceManager();
  string loc = v->getBeginLoc().printToString(sourceManager);

  if (auto vDecl = llvm::dyn_cast<VarDecl>(v->getSingleDecl())) {
    if (!vDecl->hasInit()) {
      for (unsigned int i = 0; i < injectLocs_Stmt.size(); ++i) {
        if (loc == injectLocs_Stmt[i]) {
          clang::LangOptions lo;
          string stmt_str;
          llvm::raw_string_ostream outstream(stmt_str);

          v->printPretty(outstream, NULL, PrintingPolicy(lo));
          // skip va_list
          if (stmt_str.find("va_list") != std::string::npos || stmt_str.find("[") == std::string::npos)
            return true;

          SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(v->getEndLoc(), *Context);
          SourceLocation endLoc;
          if (semiLoc == SourceLocation())
            endLoc = sourceManager.getExpansionRange(v->getEndLoc()).getEnd();
          else
            endLoc =semiLoc;

//          std::stringstream endStream;
//          endStream << "@#ifdef FRCOV@";
//          endStream << "}@";
//          endStream << "#endif";
//          operations.push_back(getInsertOp(injectLocID_Stmt[i], endLoc.printToString(sourceManager), endStream.str(), 1));


          std::stringstream offsetStream;
          offsetStream << "-FR" << injectLocID_Stmt[i];

          operations.push_back(getInsertOp(injectLocID_Stmt[i], endLocs_Stmt[i], offsetStream.str(), 0));

          std::stringstream injectStream;
          injectStream << "@#ifdef FRCOV@";
          injectStream << "int FR" << injectLocID_Stmt[i] << " = " << "FIXREVERTER[" << injectLocID_Stmt[i] << "];@";
          injectStream << "if(FIXREVERTER[" << injectLocID_Stmt[i] << "])@";
          injectStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Stmt[i] << "\\n\");@";
          injectStream << "#else@";
          injectStream << "#define FR" << injectLocID_Stmt[i] << " 1@";
          injectStream << "#endif@";
          operations.push_back(getInsertOp(injectLocID_Stmt[i], loc, injectStream.str(), 0));
        }
      }
    }
  }
  return true;
}

bool NullCheckMLVisitor::VisitValueStmt(ValueStmt *v) {
  SourceManager &sourceManager = Context->getSourceManager();
  string loc = v->getBeginLoc().printToString(sourceManager);
  string startLocStr;
  for (unsigned int i = 0; i < injectLocs_Stmt.size(); ++i) {
    if (loc == injectLocs_Stmt[i] && llvm::isa<CallExpr>(v)) {
//      unsigned n = CallExp->getNumArgs();
//      auto last_arg = CallExp->getArg(n-1);

      clang::LangOptions lo;
      string stmt_str;
      llvm::raw_string_ostream outstream(stmt_str);

      bool stmtPrinted = false;
      auto parents = Context->getParents(*v);
      if (parents.size() > 0) {
        auto parent = parents[0].get<Stmt>();
        if (parent && (llvm::isa<ReturnStmt>(parent) || llvm::isa<DeclStmt>(parent) || llvm::isa<BinaryOperator>(parent))) {
          parent->printPretty(outstream, NULL, PrintingPolicy(lo));
          startLocStr = parent->getBeginLoc().printToString(sourceManager);
          stmtPrinted = true; 
        } else if (parent && llvm::isa<CastExpr>(parent)) {
          auto grandParents = Context->getParents(*parent);
          if (grandParents.size() > 0) {
            auto grandParent = grandParents[0].get<Stmt>();
            if (grandParent && (llvm::isa<ReturnStmt>(grandParent) || llvm::isa<DeclStmt>(grandParent) || llvm::isa<BinaryOperator>(grandParent))) {
              grandParent->printPretty(outstream, NULL, PrintingPolicy(lo));
             startLocStr = grandParent->getBeginLoc().printToString(sourceManager);
              stmtPrinted = true; 
            }
          }
        }
      }

      if (!stmtPrinted) {
        v->printPretty(outstream, NULL, PrintingPolicy(lo));
        startLocStr = loc;
      }

      SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(v->getEndLoc(), *Context);
      SourceLocation endLoc;
      if (semiLoc == SourceLocation())
        endLoc = sourceManager.getExpansionRange(v->getEndLoc()).getEnd();
      else
        endLoc =semiLoc;

      std::stringstream endStream;
      endStream << "@#ifdef FRCOV@";
      endStream << "}@";
      endStream << "#endif";
      operations.push_back(getInsertOp(injectLocID_Stmt[i], endLoc.printToString(sourceManager), endStream.str(), 1));

      std::stringstream offsetStream;
      offsetStream << "-FR" << injectLocID_Stmt[i];
      operations.push_back(getInsertOp(injectLocID_Stmt[i], endLocs_Stmt[i], offsetStream.str(), 0));

      std::stringstream injectStream;
      injectStream << "@#ifdef FRCOV@";
      injectStream << "{int FR" << injectLocID_Stmt[i] << " = " << "FIXREVERTER[" << injectLocID_Stmt[i] << "];@";
      injectStream << "if(FIXREVERTER[" << injectLocID_Stmt[i] << "])@";
      injectStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Stmt[i] << "\\n\");@";
      
      injectStream << "#else@";
      injectStream << "#define FR" << injectLocID_Stmt[i] << " 1@";
      injectStream << "#endif@";
      operations.push_back(getInsertOp(injectLocID_Stmt[i], startLocStr, injectStream.str(), 0));
    }
  }
  return true;
}


bool NullCheckMLVisitor::VisitIfStmt(IfStmt *stmt)
{
  SourceManager &sourceManager = Context->getSourceManager();

  SourceLocation beginLoc = stmt->getBeginLoc();
  if (!stmt->getCond()) return true;
  SourceLocation condEndLoc = sourceManager.getExpansionRange(stmt->getCond()->getEndLoc()).getEnd();
  // we don't have else branch in matched if statements
  SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(stmt->getThen()->getEndLoc(),*Context);

  SourceLocation endLoc;
  if (semiLoc == SourceLocation())
    endLoc = sourceManager.getExpansionRange(stmt->getThen()->getEndLoc()).getEnd();
  else
    endLoc =semiLoc;

  SourceLocation bodyBeginLoc = stmt->getThen()->getBeginLoc();

  for (unsigned int i = 0; i < injectLocs_Cond.size(); ++i) {
    if (injectLocs_Cond[i] == beginLoc.printToString(sourceManager)) {
      string trace = getConds(vars_Cond[i], TRACE);
      string untrace = getConds(vars_Cond[i], UNTRACE);

      std::stringstream endStream;
      endStream << "@#ifdef FRCOV@";
      endStream << "}";
      if (!llvm::isa<CompoundStmt>(stmt->getThen()) && vars_Cond[i]["pattern"] == "COND_EXEC")
        endStream << "}";
      endStream << "@#endif";
      
      SourceLocation ifEnd;
      if (stmt->hasElseStorage()) {
        auto elseSemiLoc = arcmt::trans::findSemiAfterLocation(stmt->getElse()->getEndLoc(),*Context);
        if (elseSemiLoc == SourceLocation())
          ifEnd = sourceManager.getExpansionRange(stmt->getElse()->getEndLoc()).getEnd();
        else
          ifEnd =elseSemiLoc;
      } else
        ifEnd = endLoc;

      operations.push_back(getInsertOp(injectLocID_Cond[i], 
                          ifEnd.printToString(sourceManager), 
                          endStream.str(), 1));


      if (vars_Cond[i]["pattern"] == "COND_EXEC") {
        std::stringstream bodyStream;
        if (llvm::isa<CompoundStmt>(stmt->getThen())) {
          bodyStream << "@  @#ifdef FRCOV@";
          bodyStream << "  if (!(" << trace << "))@";
          bodyStream << "    fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
          bodyStream << "  #endif@";
          operations.push_back(getInsertOp(injectLocID_Cond[i], 
                              bodyBeginLoc.printToString(sourceManager), 
                              bodyStream.str(), 1));
        } else {
          bodyStream << "@#ifdef FRCOV@";
          bodyStream << "{if (!(" << trace << "))@";
          bodyStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
          bodyStream << "#endif@";
          operations.push_back(getInsertOp(injectLocID_Cond[i], 
                              bodyBeginLoc.printToString(sourceManager), 
                              bodyStream.str(), 0));
        }
      }

      std::stringstream condStream;
      condStream << "@#ifdef FRCOV@";
      condStream << "{if (FIXREVERTER[" << injectLocID_Cond[i] << "]) {@";
      if (vars_Cond[i]["pattern"] == "COND_EXEC")
        condStream << "  fprintf(stderr, \"reached bug index " << injectLocID_Cond[i] << "\\n\");@";
      else {
        condStream << "  if ((" << trace << ") && !(" << untrace << "))@";
        condStream << "    fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
        condStream << "  else@"; 
        condStream << "    fprintf(stderr, \"reached bug index " << injectLocID_Cond[i] << "\\n\");@";
      }
      condStream << "}@";

      condStream << "if (";
      if (untrace != "0")
        condStream << "(FIXREVERTER[" << injectLocID_Cond[i] << "]" 
           << " && (" << untrace << ")) || ";
      condStream << "(!FIXREVERTER[" 
         << injectLocID_Cond[i] << "]" << " && (" 
         << getConds(vars_Cond[i], ALL) << "))";
      condStream << "@#else";
      condStream << "@if (" << getConds(vars_Cond[i], UNTRACE);
      condStream << "@#endif@";
      operations.push_back(getReplaceOp(injectLocID_Cond[i], 
                            beginLoc.printToString(sourceManager), 
                            condEndLoc.printToString(sourceManager), condStream.str(), 0));
      }
    }
  return true;
}

bool NullCheckMLVisitor::VisitWhileStmt(WhileStmt *stmt)
{
  SourceManager &sourceManager = Context->getSourceManager();

  SourceLocation beginLoc = stmt->getBeginLoc();
  if (!stmt->getCond()) return true;
  SourceLocation condEndLoc = sourceManager.getExpansionRange(stmt->getCond()->getEndLoc()).getEnd();
  // we don't have else branch in matched if statements
  SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(stmt->getBody()->getEndLoc(),*Context);

  SourceLocation endLoc;
  if (semiLoc == SourceLocation())
    endLoc = sourceManager.getExpansionRange(stmt->getBody()->getEndLoc()).getEnd();
  else
    endLoc =semiLoc;

  SourceLocation bodyBeginLoc = stmt->getBody()->getBeginLoc();

  for (unsigned int i = 0; i < injectLocs_Cond.size(); ++i) {
    if (injectLocs_Cond[i] == beginLoc.printToString(sourceManager)) {
      string trace = getConds(vars_Cond[i], TRACE);
      string untrace = getConds(vars_Cond[i], UNTRACE);

      std::stringstream endStream;
      endStream << "@#ifdef FRCOV@";
      endStream << "}";
      if (!llvm::isa<CompoundStmt>(stmt->getBody()))
        endStream << "}";
      endStream << "@#endif";
      operations.push_back(getInsertOp(injectLocID_Cond[i], 
                          endLoc.printToString(sourceManager), 
                          endStream.str(), 1));

      std::stringstream bodyStream;

      if (llvm::isa<CompoundStmt>(stmt->getBody())) {
        bodyStream << "@  @#ifdef FRCOV@";
        bodyStream << "  if (!(" << trace << "))@";
        bodyStream << "    fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
        bodyStream << "  #endif@";
        operations.push_back(getInsertOp(injectLocID_Cond[i], 
                            bodyBeginLoc.printToString(sourceManager), 
                            bodyStream.str(), 1));
      } else {
        bodyStream << "@#ifdef FRCOV@";
        bodyStream << "{if (!(" << trace << "))@";
        bodyStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
        bodyStream << "#endif@";
        operations.push_back(getInsertOp(injectLocID_Cond[i], 
                            bodyBeginLoc.printToString(sourceManager), 
                            bodyStream.str(), 0));
      }

      std::stringstream condStream;
      condStream << "@#ifdef FRCOV@";
      condStream << "{if (FIXREVERTER[" << injectLocID_Cond[i] << "])@";
      condStream << "  fprintf(stderr, \"reached bug index " << injectLocID_Cond[i] << "\\n\");@";

      condStream << "while (";
      if (untrace != "0")
        condStream << "(FIXREVERTER[" << injectLocID_Cond[i] << "]" 
           << " && (" << untrace << ")) || ";
      condStream << "(!FIXREVERTER[" 
         << injectLocID_Cond[i] << "]" << " && (" 
         << getConds(vars_Cond[i], ALL) << "))";
      condStream << "@#else";
      condStream << "@while (" << getConds(vars_Cond[i], UNTRACE);
      condStream << "@#endif@";
      operations.push_back(getReplaceOp(injectLocID_Cond[i], 
                            beginLoc.printToString(sourceManager), 
                            condEndLoc.printToString(sourceManager), condStream.str(), 0));
    }
  }
  return true;
}

bool NullCheckMLVisitor::VisitForStmt(ForStmt *stmt)
{
  SourceManager &sourceManager = Context->getSourceManager();

  SourceLocation beginLoc = stmt->getBeginLoc();
  if (!stmt->getCond()) return true;
  SourceLocation condEndLoc = sourceManager.getExpansionRange(stmt->getCond()->getEndLoc()).getEnd();
  // we don't have else branch in matched if statements
  SourceLocation semiLoc = arcmt::trans::findSemiAfterLocation(stmt->getBody()->getEndLoc(), *Context);

  SourceLocation endLoc;
  if (semiLoc == SourceLocation())
    endLoc = sourceManager.getExpansionRange(stmt->getBody()->getEndLoc()).getEnd();
  else
    endLoc =semiLoc;

  SourceLocation bodyBeginLoc = stmt->getBody()->getBeginLoc();

  for (unsigned int i = 0; i < injectLocs_Cond.size(); ++i) {
    if (injectLocs_Cond[i] == beginLoc.printToString(sourceManager)) {

      std::string init_str = "";
      if (stmt->getInit()) {
        llvm::raw_string_ostream init_stream(init_str);
        clang::LangOptions lo;
        stmt->getInit()->printPretty(init_stream, NULL, PrintingPolicy(lo));
      }

      string trace = getConds(vars_Cond[i], TRACE);
      string untrace = getConds(vars_Cond[i], UNTRACE);

      std::stringstream endStream;
      endStream << "@#ifdef FRCOV@";
      endStream << "}";
      if (!llvm::isa<CompoundStmt>(stmt->getBody()))
        endStream << "}";
      endStream << "@#endif";
      operations.push_back(getInsertOp(injectLocID_Cond[i], 
                          endLoc.printToString(sourceManager), 
                          endStream.str(), 1));

      std::stringstream bodyStream;
      if (llvm::isa<CompoundStmt>(stmt->getBody())) {
        bodyStream << "@  @#ifdef FRCOV@";
        bodyStream << "  if (!(" << trace << "))@";
        bodyStream << "    fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
        bodyStream << "  #endif@";
        operations.push_back(getInsertOp(injectLocID_Cond[i], 
                            bodyBeginLoc.printToString(sourceManager), 
                            bodyStream.str(), 1));
      } else {
        bodyStream << "@#ifdef FRCOV@";
        bodyStream << "{if (!(" << trace << "))@";
        bodyStream << "  fprintf(stderr, \"triggered bug index " << injectLocID_Cond[i] << "\\n\");@";
        bodyStream << "#endif@";
        operations.push_back(getInsertOp(injectLocID_Cond[i], 
                            bodyBeginLoc.printToString(sourceManager), 
                            bodyStream.str(), 0));
      }

      std::stringstream condStream;
      condStream << "@#ifdef FRCOV@";
      condStream << "{if (FIXREVERTER[" << injectLocID_Cond[i] << "])@";
      condStream << "  fprintf(stderr, \"reached bug index " << injectLocID_Cond[i] << "\\n\");@";

      condStream << "for (" << init_str << "; ";
      if (untrace != "0")
        condStream << "(FIXREVERTER[" << injectLocID_Cond[i] << "]" 
           << " && (" << untrace << ")) || ";
      condStream << "(!FIXREVERTER[" 
         << injectLocID_Cond[i] << "]" << " && (" 
         << getConds(vars_Cond[i], ALL) << "))";
      condStream << "@#else";
      condStream << "@for ("  << init_str << "; " << getConds(vars_Cond[i], UNTRACE);
      condStream << "@#endif@";
      operations.push_back(getReplaceOp(injectLocID_Cond[i], 
                            beginLoc.printToString(sourceManager), 
                            condEndLoc.printToString(sourceManager), condStream.str(), 0));
    }
  }
  return true;
}

string NullCheckMLVisitor::getConds(json j, CondType type) {
  string ret = "";
  vector<json> toks = j["tokenOrder"];
  vector<json> ids = j["ids"];
  vector<int> subExprs;
  vector<int> traceVars;

  for (json id : ids) {
    if (id["tag"] == "logic[]")
      subExprs.push_back(id["order"]);

    if (((string) id["tag"]).find("traceVar") != string::npos && ((bool) id["crash"]))
        traceVars.push_back(id["order"]);
  }

  int tokEndOffset = 2;
  for (json tok : toks) {
    if (tok["val"] == "else")
      tokEndOffset = 4;
  }
  subExprs.push_back(j["tokenOrder"].size() - tokEndOffset);

  std::sort(subExprs.begin(),subExprs.end());

  int start = 1;
  for (int order : subExprs) {
    //check from start to end, if a traceVar exists,
    //add entire subexpr to ret
    int end = order;
    bool hasTrace = false;

    for (int traceOrder : traceVars) {
      if (traceOrder >= start && traceOrder < end) {
        hasTrace = true;
        if (type == TRACE || type == ALL) {
          for (int k = (ret == "" && start > 1 ? start+1 : start); k < end; ++k)
            ret += getVar(toks,k) + " ";
        }
        break;
      }
    }

    if (!hasTrace && (type == UNTRACE || type == ALL)) {
      for (int k = (ret == "" && start > 1 ? start+1 : start); k < end; ++k)
        ret += getVar(toks,k) + " ";
    }
    start = order;
  }
  if (ret == "")
    ret = "0";
  return ret;
}

string NullCheckMLVisitor::getVar(vector<json> js, int x)
{
  for (unsigned int i = 0; i < js.size(); ++i)
    if (js[i]["order"] == x)
      return ((string)js[i]["val"]);
  return "";
}
