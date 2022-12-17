#ifndef TAINTNODE_H
#define TAINTNODE_H

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"

class TaintNode {
public:
  virtual ~TaintNode(){}

  virtual bool operator==(const TaintNode& t) = 0;

  virtual bool operator<(const TaintNode& t) = 0;

  virtual std::size_t hashCode() = 0;
};

class TaintRootNode : public TaintNode {
public:
  std::size_t hashCode();

  const llvm::Value * Value;

protected:
  TaintRootNode(const llvm::Value* Inst) : Value(Value) {}
}

class GlobalRootNode : public TaintRootNode {
public:
  GlobalRootNode(const llvm::Value*);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);
};

class LocalRootNode : public TaintRootNode {
public:
  FieldBaseNodeWithEdge(const llvm::Value*, const llvm::Instruction*);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);
};

class TaintNodeWithEdge : public TaintNode {
public:
  const llvm::Instruction* Inst;

protected:
  TaintNodeWithEdge(const llvm::Instruction* Inst) : Inst(Inst) {}
}

class ValueNodeWithEdge : public TaintNodeWithEdge {
public:
  ValueNodeWithEdge(const llvm::Instruction*, const llvm::Value *);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);

  std::size_t hashCode();

  const llvm::Value * Value;
};

class FieldBaseNodeWithEdge : public TaintNodeWithEdge {
public:
  FieldBaseNodeWithEdge(const llvm::Instruction*, const llvm::PointerType *, int);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);

  std::size_t hashCode();

  const llvm::PointerType *Field;
  int Base;
};

template<class T> struct ptr_less {
  bool operator() (const std::shared_ptr<T> lhs, const std::shared_ptr<T> rhs) const {
    return *lhs < *rhs;
  }
};

struct TaintNodeHash {
  std::size_t operator()(std::shared_ptr<TaintNode> k) const {
    return k->hashCode();
  }
};

struct TaintNodeEquality {
  bool operator()(std::shared_ptr<TaintNode> lhs, std::shared_ptr<TaintNode> rhs) const {
    return *lhs == *rhs;
  }
};

#endif
