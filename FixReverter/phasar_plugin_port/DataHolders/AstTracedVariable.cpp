#include "AstTracedVariable.h"

#include <boost/functional/hash.hpp>

std::unordered_set<std::string> TracedVarTypes = {"traceVarNullCheck[]", "traceVarNumCmp[]", "traceVarPtrRange[]", "traceVarAssignNull[]", "traceVarAssignNum[]"};
std::unordered_set<std::string> TracedVarBaseTypes = {"traceVarNullCheckBase[]", "traceVarNumCmpBase[]", "traceVarPtrRangeBase[]", "traceVarAssignNullBase[]", "traceVarAssignNumBase[]"};
std::unordered_set<std::string> CondAssignVarTypes = {"traceVarAssignNull[]", "traceVarAssignNum[]", "traceVarAssignNullBase[]", "traceVarAssignNumBase[]"};
AstTracedVariable::AstTracedVariable(int index, int iid, 
                                        std::string pattern, 
                                        std::string traceType, 
                                        SourceInfo DecSrcInfo,
                                        int jsonIndex)
                                             : index(index), iid(iid), pattern(pattern), traceType(traceType), 
                                               DecSrcInfo(DecSrcInfo),
                                               jsonIndex(jsonIndex)
{
  DecValue = NULL;
  ReachedCount = 0;
  fieldOffset = -1;
  BodyStart = std::nullopt;
  BodyEnd = std::nullopt;
}

AstTracedVariable::~AstTracedVariable() {}

bool AstTracedVariable::decMatch(int line, int col) const {
  return line == DecSrcInfo.line && col == DecSrcInfo.col;
}

bool AstTracedVariable::decMatch(int line) const {
  return line == DecSrcInfo.line;
}

bool AstTracedVariable::isReached() const {
  return ReachedCount > 0;
}

void AstTracedVariable::incrementReachedCount() {
  ReachedCount++;
}

bool AstTracedVariable::operator==(const AstTracedVariable &other) const { 
  return index == other.index && iid == other.iid;
}

std::size_t AstTracedVariable::hashCode() const{
  std::size_t seed = 0;
  boost::hash_combine(seed, index);
  boost::hash_combine(seed, iid);
  return seed;
}


