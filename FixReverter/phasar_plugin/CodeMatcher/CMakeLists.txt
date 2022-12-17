include_directories (${PROJECT_SOURCE_DIR})
include_directories (${Boost_INCLUDE_DIR})
include_directories(${LLVM_INCLUDE_DIRS})

add_library(CodeMatcher SHARED
  CodeMatcher.cpp
)
