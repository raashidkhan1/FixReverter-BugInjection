#include "TaintNode.h"
#include <iostream>
#include <boost/functional/hash.hpp>

FieldNode::FieldNode(const llvm::PointerType* Field, int Base) : Field(Field), Base(Base) {}

bool FieldNode::operator==(const TaintNode& other) {
  if (auto TF = dynamic_cast<const FieldNode*>(&other))
      return Field == TF->Field && Base == TF->Base;
  return false;
}

bool FieldNode::operator<(const TaintNode& other) {
  if (auto TV = dynamic_cast<const ValueNode*>(&other)) 
    return true;
  else if (auto TF = dynamic_cast<const FieldNode*>(&other)) {
    if (Field < TF->Field)
      return true;
    else if (Field > TF->Field)
      return false;
    return Base < TF->Base;
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

std::size_t FieldNode::hashCode() {
  std::size_t seed = 0;
  boost::hash_combine(seed, Field);
  boost::hash_combine(seed, Base);
  return seed;
}

ValueNode::ValueNode(const llvm::Value* Value) : Value(Value) {}

bool ValueNode::operator==(const TaintNode& other) {
  if (auto TV = dynamic_cast<const ValueNode*>(&other))
      return Value == TV->Value;
  return false;
}

bool ValueNode::operator<(const TaintNode& other) {
  if (auto TF = dynamic_cast<const FieldNode*>(&other))
    return false;
  else if (auto TV = dynamic_cast<const ValueNode*>(&other)) {
    return Value < TV->Value;
  }
  std::cout << "something is wrong" << std::endl;
  exit(0);
}

std::size_t ValueNode::hashCode() {
  std::hash<const llvm::Value *> hashFunc;
  return hashFunc(Value);
}



