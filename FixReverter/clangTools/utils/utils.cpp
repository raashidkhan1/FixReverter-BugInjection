#include "utils.h"

namespace fs = boost::filesystem;

fs::path getRootDir(std::string rootDirStr) {
  fs::path rootDir(rootDirStr);
  if (rootDir.is_relative()) {
    llvm::errs() << "[ERROR] - root directory path is not an absolute path: " << rootDir.string() << "\n";
    exit(0);
  } 
  if (!fs::is_directory(rootDir)) {
    llvm::errs() << "[ERROR] - root directory path is not a directory: " << rootDir.string() << "\n";
    exit(0);
  }
  return rootDir;
}

fs::path getSrcPath(const std::vector<std::string> srcPathList) {
  //validate input number and format
  if (srcPathList.size() != 1) {
    llvm::errs() << "[ERROR] - this tool takes exactly one file as input" << "\n";
    exit(0);
  }
  fs::path srcPath(srcPathList[0]);
  if (srcPath.is_relative()) {
    llvm::errs() << "[ERROR] - input file must be given as absolute path" << "\n";
    exit(0);
  }
  if (!fs::is_regular_file(srcPath)) {
    llvm::errs() << "[ERROR] - input path is not a file" << "\n";
    exit(0);
  }
  return srcPath;
}

std::pair<fs::path, fs::path> getCommandDirFilePair(clang::tooling::CompilationDatabase &CompileDB,
                                               const std::vector<std::string> &srcPathList) {
  //srcPathList has been checked to have size of 1
  std::vector<clang::tooling::CompileCommand> Commands = CompileDB.getCompileCommands(srcPathList[0]);

  if (Commands.empty()) {
    llvm::errs() << "[ERROR] - compile command not found" << "\n";
    exit(0);
  }
  // llvm::errs() << Commands[0].Heuristic << "\n";
  std::string commandDirStr = Commands[0].Directory;
  std::string fileName = Commands[0].Filename;
  //if no compiliation database is found commandDirStr will be invalid and fileName will be absolute(enfored by input)
  fs::path commandDir(Commands[0].Directory);
  fs::path fileNamePath(Commands[0].Filename);
  return std::make_pair(commandDir, fileNamePath);
}

std::pair<std::string, std::string> getCommandDirFileStrPair(clang::tooling::CompilationDatabase &CompileDB,
                                               const std::vector<std::string> &srcPathList) {
  std::pair<fs::path, fs::path> commandDirFilePair = getCommandDirFilePair(CompileDB, srcPathList);
  return std::make_pair(commandDirFilePair.first.string(), commandDirFilePair.second.string());
}

const std::vector<std::string> stringSplit(std::string str, char delimiter) {
  std::vector<std::string> elems;
  std::stringstream ss(str);
  std::string elem;

  while(getline(ss, elem, delimiter))
    elems.push_back(elem);
  return elems;
}
