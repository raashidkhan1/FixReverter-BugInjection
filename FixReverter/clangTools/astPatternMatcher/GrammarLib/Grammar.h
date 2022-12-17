#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include "PatternToken.h"
#include "Terminal.h"
#include "NonTerminal.h"
#include "Action.h"
#include "NameAction.h"
#include "IDAction.h"
#include "ArrayAction.h"
#include "State.h"

using std::vector;
using std::string;
using std::pair;

#define TERMINALS							\
  {"t_if","t_fi","t_else","t_condThen","t_condElse","t_compoundBegin","t_compoundEnd","t_less3", \
      "t_var","t_ptrVar","t_const","t_num","t_null","t_litFloat","t_litNum", \
      "t_break","t_goto","t_return","t_continue","t_function","t_sizeof", \
      "t_shiftL","t_shiftR","t_flip","t_bitAnd","t_bitOr","t_bitXor",	\
      "t_plus","t_minus","t_mult","t_div","t_mod","t_inc","t_dec","t_pos","t_neg", \
      "t_not","t_eqEq","t_nEq","t_and","t_or","t_les","t_gtr","t_lesEq","t_gtrEq", \
      "t_deref","t_ref","t_fieldAcc", "t_derefFieldAcc","t_brackL","t_brackR","t_cast",\
      "t_assign","t_comma", "t_parenL", "t_parenR", "t_for", "t_rof", "t_while", "t_elihw", \
      "t_semi", "t_type", "t_noJump", "t_asnStmt" }

class Grammar
{
 public:
  Grammar(string patternFile);
  string toString();
  State* getRoot();
 private:
  
  bool accepted;
  bool valid;
  int state;
  vector<PatternToken*> tokens;
  State *initialState;

  void setTerminals();
  void parseGrammar(string patternFile);
  void constructSets();
  vector<PatternToken*> getBeginSet(PatternToken* t);
  void getFollowSet(PatternToken* t);
  void createStateGraph();
  Action* determineAction(std::string s);
};
