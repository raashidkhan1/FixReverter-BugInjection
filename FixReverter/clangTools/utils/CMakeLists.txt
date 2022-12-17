set(LLVM_LINK_COMPONENTS support)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS filesystem system REQUIRED)
include_directories (SYSTEM ${Boost_INCLUDE_DIR})

include_directories (${PROJECT_SOURCE_DECLARATION_DIRECTORY})

add_library(utils
  utils.cpp utils.h
  )

target_link_libraries(utils
  PRIVATE
  clangTooling
  clangBasic
  clangSerialization
  LLVMSupport
  # ${Boost_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )
