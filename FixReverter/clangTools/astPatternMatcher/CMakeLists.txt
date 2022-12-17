set (LLVM_LINK_COMPONENTS support)

add_subdirectory(GrammarLib)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../utils)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann)

add_clang_executable(astPatternMatcher
  APMmain.cpp
  Pattern.h
  tokenizer.cpp tokenizer.h
  )

target_link_libraries(astPatternMatcher 
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangRewrite
  clangTooling
  clangSerialization
  grammarLib
  utils
  )
