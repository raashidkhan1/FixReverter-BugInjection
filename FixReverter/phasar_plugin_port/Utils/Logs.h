#ifndef FLOWLOGS_H
#define FLOWLOGS_H

#include <set>
#include <map>
#include <string>

#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include "Configs.h"
#include "DataHolders/AstTracedVariable.h"


void logFlowFunction(llvm::raw_ostream &, const llvm::Value*, const std::string);

void logFromToFlow(llvm::raw_ostream &, const llvm::Value*, const llvm::Value*, const std::string);

void logFromToFlow(llvm::raw_ostream &, const llvm::Value*, llvm::StringRef, const std::string);

void logTaintSources(llvm::raw_ostream &, std::set<AstTracedVariable *>);

void logGeneralInfo(llvm::raw_ostream &, const std::string);

void logGeneralInfoWithValue(llvm::raw_ostream &, const std::string, const llvm::Value*);

#endif
