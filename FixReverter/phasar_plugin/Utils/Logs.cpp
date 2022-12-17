#include "Logs.h"

void logFlowFunction(llvm::raw_ostream &os, const llvm::Value* value, const std::string type) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32mAnalyzer - [DEBUG] ";
    os <<  type;
    os << " flow function: ";
    value->print(os);
    os << "\033[0m\n";
  }
}

void logFromToFlow(llvm::raw_ostream &os, const llvm::Value* from, const llvm::Value* to, const std::string type) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32mAnalyzer - [DEBUG] ";
    os <<  type;
    os << " flow: from ";
    from->print(os);
    os << "\nAnalyzer - [DEBUG] ";
    os <<  type;
    os << " flow: to ";
    to->print(os);
    os<< "\033[0m\n";
  }
}

void logFromToFlow(llvm::raw_ostream &os, const llvm::Value* from, llvm::StringRef to, const std::string type) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32mAnalyzer - [DEBUG] ";
    os <<  type;
    os << " flow: from ";
    from->print(os);
    os << "\nAnalyzer - [DEBUG] ";
    os <<  type;
    os << " flow: to " << to << " \033[0m\n";
  }
}

void logTaintSources(llvm::raw_ostream &os, std::set<AstTracedVariable *> Sources) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32mAnalyzer - [DEBUG] sources are\n";
    for (auto S : Sources) {
      S->getDecValue()->print(os);
      os << "\n";
    }
    os << "\033[0m\n";
  }
}

void logGeneralInfo(llvm::raw_ostream &os, const std::string msg) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32m";
    os << msg;
    os << "\033[0m\n";
  }
}

void logGeneralInfoWithValue(llvm::raw_ostream &os, const std::string msg, const llvm::Value* value) {
  if (LOG_DEBUG_INFO) {
    os << "\033[1;32m";
    os << msg;
    value->print(os);
    os << "\033[0m\n";
  }
}
