#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"
#include "Transforms.h"
#include "json.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using std::string;
using std::vector;
using json = nlohmann::json;
extern vector<json> operations;

string getPadding(string start, string end, int def);

json getInsertOp(int, string, string, int);

json getReplaceOp(int, string, string, string, int);
