set(LLVM_LINK_COMPONENTS support)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories (SYSTEM ${Boost_INCLUDE_DIR})
include_directories (${PROJECT_SOURCE_DECLARATION_DIRECTORY})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../utils)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../nlohmann)

add_clang_executable(mainFuncLocator
  mainFuncLocator.cpp
  )
target_link_libraries(mainFuncLocator
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  clangAST
  clangFrontend
  clangSerialization
  # ${Boost_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  utils
  )
