cmake_minimum_required(VERSION 3.12)

project(DependenceAnalyzer)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(DependenceAnalyzer SHARED
  DependenceAnalyzer.cpp
)

find_package(LLVM REQUIRED IR CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
llvm_map_components_to_libnames(llvm_libs ir)
include_directories(${LLVM_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system container REQUIRED)
include_directories (SYSTEM ${Boost_INCLUDE_DIR})

find_package(phasar COMPONENTS ifdside REQUIRED)
include_directories(${PHASAR_INCLUDE_DIR})


include_directories(".")

target_link_libraries(DependenceAnalyzer PUBLIC
  DataHolders
  PRIVATE
  Utils
  AstPatternParser
  CodeMatcher
  OutputWriter
  )

add_subdirectory(Utils)
add_subdirectory(DataHolders)
add_subdirectory(AstPatternParser)
add_subdirectory(CodeMatcher)
add_subdirectory(OutputWriter)
