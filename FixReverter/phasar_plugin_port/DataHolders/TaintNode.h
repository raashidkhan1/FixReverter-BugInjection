#ifndef TaintNode_H
#define TaintNode_H

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"

class TaintNode {
public:
  virtual ~TaintNode(){}

  virtual bool operator==(const TaintNode& t) = 0;

  virtual bool operator<(const TaintNode& t) = 0;

  virtual std::size_t hashCode() = 0;
};

class FieldNode : public TaintNode {
public:
  FieldNode(const llvm::PointerType *, int);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);

  std::size_t hashCode();

  const llvm::PointerType *Field;
  int Base;
};

class ValueNode : public TaintNode {
public:
  ValueNode(const llvm::Value *);

  bool operator==(const TaintNode& other);

  bool operator<(const TaintNode& other);

  std::size_t hashCode();

  const llvm::Value * Value;
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
