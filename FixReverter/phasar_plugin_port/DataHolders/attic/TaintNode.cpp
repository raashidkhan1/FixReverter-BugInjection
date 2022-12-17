#include "TaintNode.h"
#include <iostream>
#include <boost/functional/hash.hpp>

// TaintRootNode
std::size_t TaintRootNode::hashCode() {
  std::size_t seed = 0;
  boost::hash_combine(seed, Value);
  return seed;
}

// GlobalRootNode
GlobalRootNode::GlobalRootNode(const llvm::Value* Value) : TaintRootNode(Value) {}

bool GlobalRootNode::operator==(const TaintNode& other) {
  if (auto GRN = dynamic_cast<const GlobalRootNode*>(&other))
      return Value == GRN->Value;
  return false;
}

bool GlobalRootNode::operator<(const TaintNode& other) {
  if (auto TNE = dynamic_cast<const TaintNodeWithEdge*>(&other)) 
    return false;
  if (auto TRN = dynamic_cast<const TaintRootNode*>(&other)) {
    if (auto GRN = dynamic_cast<const GlobalRootNode*>(TRN)) {
      return Value < GRN->Value;
    return false;
    }
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

// LocalRootNode
LocalRootNode::LocalRootNode(const llvm::Value* Value, const llvm::Instruction* Inst) 
                                                : TaintRootNode(Value), Inst(Inst) {}

bool LocalRootNode::operator==(const TaintNode& other) {
  if (auto LRN = dynamic_cast<const LocalRootNode*>(&other))
      return Value == LRN->Value;
  return false;
}

bool LocalRootNode::operator<(const TaintNode& other) {
  if (auto TNE = dynamic_cast<const TaintNodeWithEdge*>(&other)) 
    return false;
  if (auto TRN = dynamic_cast<const TaintRootNode*>(&other)) {
    if (auto LRN = dynamic_cast<const LocalRootNode*>(TRN)) {
      return Value < LRN->Value;
    return true;
    }
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

// ValueNodeWithEdge
ValueNodeWithEdge::ValueNodeWithEdge(const llvm::Instruction* Inst, const llvm::Value* Value) 
    : TaintNodeWithEdge(Inst), Value(Value) {}

bool ValueNodeWithEdge::operator==(const TaintNode& other) {
  if (auto TV = dynamic_cast<const ValueNodeWithEdge*>(&other))
      return Value == TV->Value && Inst == TV->Inst;
  return false;
}

bool ValueNodeWithEdge::operator<(const TaintNode& other) {
  if (auto TN = dynamic_cast<const FieldBaseNodeWithEdge*>(&other))
    return false;
  else if (auto TV = dynamic_cast<const ValueNodeWithEdge*>(&other)) {
    if (Inst != TV->Inst)
      return Inst < TV->Inst;
    return Value < TV->Value;
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

std::size_t ValueNodeWithEdge::hashCode() {
  std::size_t seed = 0;
  boost::hash_combine(seed, Inst);
  boost::hash_combine(seed, Value);
  return seed;
}

// FieldBaseNodeWithEdge
FieldBaseNodeWithEdge::FieldBaseNodeWithEdge(const llvm::Instruction* Inst,
                                                const llvm::PointerType* Field, int Base) 
                                                : TaintNodeWithEdge(Inst), Field(Field), Base(Base) {}

bool FieldBaseNodeWithEdge::operator==(const TaintNode& other) {
  if (auto FBNE = dynamic_cast<const FieldBaseNodeWithEdge*>(&other))
      return Inst == FBNE->Inst && Field == FBNE->Field && Base == FBNE->Base;
  return false;
}

bool FieldBaseNodeWithEdge::operator<(const TaintNode& other) {
  if (auto TRN = dynamic_cast<const TaintRootNode*>(TRN))
    return true;
  if (auto TNE = dynamic_cast<const TaintNodeWithEdge*>(&other)) {
    if (auto FBNE = dynamic_cast<const FieldBaseNodeWithEdge*>(&other)) {
      if (Inst < TN->Inst)
        return true;
      if (Inst > TN->Inst)
        return false;
      if (Field < TN->Field)
       return true;
      if (Field > TN->Field)
        return false;
      return Base < TN->Base;
    }
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

std::size_t FieldBaseNodeWithEdge::hashCode() {
  std::size_t seed = 0;
  boost::hash_combine(seed, Inst);
  boost::hash_combine(seed, Field);
  boost::hash_combine(seed, Base);
  return seed;
}
