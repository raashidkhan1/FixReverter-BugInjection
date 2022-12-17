find_package(LLVM REQUIRED CONFIG)

include_directories (${PROJECT_SOURCE_DIR})
include_directories(${LLVM_INCLUDE_DIRS})

add_library(DataHolders SHARED
  AstTracedVariable.cpp
  TaintNode.cpp
)
