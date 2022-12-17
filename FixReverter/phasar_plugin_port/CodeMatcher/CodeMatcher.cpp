#include "CodeMatcher.h"

#include <boost/filesystem.hpp>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SMLoc.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

namespace fs = boost::filesystem;

std::shared_ptr<FuncDataHolder> matchSourceCode(const llvm::Function* F,
                            std::map<const std::string,
                            std::set<std::shared_ptr<AstTracedVariable>>> &InputDecMap) {
  std::shared_ptr<FuncDataHolder> ret = std::make_shared<FuncDataHolder>();
  std::vector<std::shared_ptr<AstTracedVariable>> conExeInputs;

  // iterate all global vars
  for (auto &Global : F->getParent()->getGlobalList()) {
    if (auto MN = Global.getMetadata(llvm::LLVMContext::MD_dbg)) {
      if (auto DIGVExp = llvm::dyn_cast<llvm::DIGlobalVariableExpression>(MN)) {
        auto DIGV = DIGVExp->getVariable();
        int line = DIGV->getLine();
        fs::path DecDir(DIGV->getDirectory().str());
        fs::path DecFile(DIGV->getFilename().str());
        fs::path RealDecPath = fs::canonical(DecDir / DecFile);

        if (InputDecMap.count(RealDecPath.string()) > 0) {
          for (auto input : InputDecMap[RealDecPath.string()]) {
            if (input->decMatch(line)) {
              input->incrementReachedCount();
              auto GlobalTaintValue = std::make_shared<ValueNode>(&Global);
              input -> setDecValue(&Global);
              ret->DecToTraceMap[GlobalTaintValue].insert(input);
              ret->GlobalVars.insert(&Global);

              if (input->isCondExe())
                conExeInputs.push_back(input);
            }
          }
        }
      }
    }
  }

  bool HasPrinted = false;

  for (auto &BB : *F) {
    for (auto &I : BB) {
      if (I.getMetadata(llvm::LLVMContext::MD_dbg)) {
        auto &DILoc = I.getDebugLoc();
        int line = DILoc.getLine();
        int col = DILoc.getCol();

        if (auto Scope = DILoc->getScope()) {
          fs::path DecDir(Scope->getDirectory().str());
          fs::path DecFile(Scope->getFilename().str());
          fs::path RealDecPath = fs::canonical(DecDir / DecFile);
          std::string RealDecPathStr = RealDecPath.string();

          if (!HasPrinted)
            HasPrinted = true;
          

          if (InputDecMap.count(RealDecPathStr) > 0) {
            for (auto input : InputDecMap.at(RealDecPathStr)) {
              if (input->decMatch(line, col)) {
                if (llvm::isa<llvm::DbgValueInst>(&I)) {
                  llvm::outs() << "\033[1;32mMatcher - " << "[ERROR] " << "DbgValueInst currently not supported, please compile target program with -O0 option" << "\033[0m\n";
                  exit(0);
                } else if (auto DecInst = llvm::dyn_cast<llvm::DbgDeclareInst>(&I)) {
                  input->incrementReachedCount();
                  llvm::Metadata *Meta = llvm::cast<llvm::MetadataAsValue>(DecInst->getOperand(0))->getMetadata();
                  if (llvm::isa<llvm::ValueAsMetadata>(Meta)) {
                    llvm::Value *MetaValue = llvm::cast <llvm::ValueAsMetadata>(Meta)->getValue();
                    input->setDecValue(MetaValue);
                    auto MetaTaintValue = std::make_shared<ValueNode>(MetaValue);
                    ret->DecToTraceMap[MetaTaintValue].insert(input);
                    ret->DecAllocs.insert(MetaValue);

                    if (input->isCondExe())
                      conExeInputs.push_back(input);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  for (auto &BB : *F) {
    for (auto &I : BB) {
      if (I.getMetadata(llvm::LLVMContext::MD_dbg)) {
        auto &DILoc = I.getDebugLoc();
        int line = DILoc.getLine();
        int col = DILoc.getCol();
        if (auto Scope = DILoc->getScope()) {
          fs::path InstDir(Scope->getDirectory().str());
          fs::path InstFile(Scope->getFilename().str());
          fs::path RealInstPath = fs::canonical(InstDir / InstFile);
          std::string RealInstPathStr = RealInstPath.string();

          for (auto input : conExeInputs) {
            input->setRangeFunc(F);

            SourceInfo BodyStart = input->getBodyStart();
            SourceInfo BodyEnd = input->getBodyEnd();

            if (RealInstPathStr == BodyStart.file) {
              if ((line > BodyStart.line && line < BodyEnd.line) 
                    || (line == BodyStart.line && col >= BodyStart.col)
                    || (line == BodyEnd.line && col <= BodyEnd.col)) {
                input->addInstsInRange(&I);
              }
            }
          }
        }
      }
    }
  }
  return ret;
}
