include_directories (${PROJECT_SOURCE_DIR})

include_directories (${Boost_INCLUDE_DIR})

add_library(AstPatternParser SHARED
  AstPatternParser.cpp
)

target_link_libraries(AstPatternParser PUBLIC
  DataHolders
  )
