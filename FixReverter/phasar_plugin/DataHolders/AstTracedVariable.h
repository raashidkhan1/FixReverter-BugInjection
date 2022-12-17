#ifndef ASTTRACEDVARIABLE_H_
#define ASTTRACEDVARIABLE_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_set>

#include "nlohmann/json.hpp"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "SourceInfo.h"

extern std::unordered_set<std::string> TracedVarTypes;
extern std::unordered_set<std::string> TracedVarBaseTypes;
extern std::unordered_set<std::string> CondAssignVarTypes;

class AstTracedVariable
{
  private:
  const int index;
  const int iid;    //identifier id
  const std::string pattern;
  const std::string traceType;
  const SourceInfo DecSrcInfo;    //path to declaraction of traced variable
  std::optional<SourceInfo> BodyStart;
  std::optional<SourceInfo> BodyEnd;
  // stores declare inst
  const llvm::Value *DecValue;
  // stores if statement
  int ReachedCount;
  const int jsonIndex;    //original json data
  int fieldOffset;
  std::unordered_set<const llvm::Instruction *> InstsInRange;
  const llvm::Function *RangeFunc;

  public:
  AstTracedVariable(int, int, std::string, std::string, SourceInfo, int);

  ~AstTracedVariable();
  //getters
  const int getIndex() const {return index;}

  const int getIdentifierID() const {return iid;}

  const std::string getTraceType() const {return traceType;}

  const std::string getPattern() const {return pattern;}

  const SourceInfo getDecSrcInfo() const {return DecSrcInfo;}

  const SourceInfo getBodyStart() const {return BodyStart.value();}

  const SourceInfo getBodyEnd() const {return BodyEnd.value();}

  const int getJsonIndex() const {return jsonIndex;}

  const llvm::Value * getDecValue() const {return DecValue;}

  const int getFieldOffset() {return fieldOffset;};

  const llvm::Function * getRangeFunc() {return RangeFunc;};

  //setters
  void setFieldOffset(const int offset) {fieldOffset = offset;}

  void setRangeFunc(const llvm::Function *Func) {RangeFunc = Func;}

  void setDecValue(const llvm::Value *v) {DecValue = v;}

  void setBodyStart(SourceInfo start) {BodyStart.emplace(start);};

  void setBodyEnd(SourceInfo end) {BodyEnd.emplace(end);};

  bool decMatch(int, int) const;

  bool decMatch(int) const;

  void incrementReachedCount();

  void addInstsInRange(const llvm::Instruction * Inst) {InstsInRange.insert(Inst);}

  std::unordered_set<const llvm::Instruction *> getInstsInRange() {return InstsInRange;}

  bool isReached() const;

  const bool isField() const {return TracedVarBaseTypes.count(traceType);};

  const bool isCondExe() const {return pattern == "COND_EXEC";};

  bool operator==(const AstTracedVariable &) const;

  std::size_t hashCode() const;
};

struct AstTracedVarHash {
  std::size_t operator()(std::shared_ptr<AstTracedVariable> v) const {
    return v->hashCode();
  }
};

struct AstTracedVarEquality {
  bool operator()(std::shared_ptr<AstTracedVariable> lhs, std::shared_ptr<AstTracedVariable> rhs) const {
    return *lhs == *rhs;
  }
};


#endif
