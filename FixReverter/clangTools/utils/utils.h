#ifndef CLANGTOOLSUTILS_H
#define CLANGTOOLSUTILS_H

#include <string>
#include <utility>
#include <sstream>
#include <vector>
#include <boost/filesystem.hpp>

#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/Support/CommandLine.h"

boost::filesystem::path getRootDir(std::string);

boost::filesystem::path getSrcPath(const std::vector<std::string>);

std::pair<boost::filesystem::path, boost::filesystem::path> getCommandDirFilePair(clang::tooling::CompilationDatabase&,
                                               const std::vector<std::string>&);

std::pair<std::string, std::string> getCommandDirFileStrPair(clang::tooling::CompilationDatabase&,
                                               const std::vector<std::string>&);

const std::vector<std::string> stringSplit(std::string, char);

#endif
