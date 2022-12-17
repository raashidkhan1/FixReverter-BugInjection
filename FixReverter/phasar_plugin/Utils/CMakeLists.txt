find_package(LLVM REQUIRED CONFIG)

include_directories (${PROJECT_SOURCE_DECLARATION_DIRECTORY})
include_directories(${LLVM_INCLUDE_DIRS})

add_library(Utils SHARED
  Utils.cpp
  Logs.cpp
  Configs.h
)

target_link_libraries(Utils PRIVATE
  DataHolders
  )
