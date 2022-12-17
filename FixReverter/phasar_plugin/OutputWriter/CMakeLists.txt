include_directories(${LLVM_INCLUDE_DIRS})
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${Boost_INCLUDE_DIR})
include_directories(${PHASAR_INCLUDE_DIR})

add_library(OutputWriter SHARED
  OutputWriter.cpp
  SinkTracer.cpp
)

target_link_libraries(OutputWriter PRIVATE
  DataHolders
)
