get_filename_component(PARENT_DIR ../ ABSOLUTE)
include_directories(${PARENT_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../../nlohmann)

add_library(grammarLib
  Grammar.cpp Grammar.h
  PatternToken.cpp PatternToken.h
  Terminal.cpp Terminal.h
  NonTerminal.cpp NonTerminal.h
  Action.cpp Action.h
  ArrayAction.cpp ArrayAction.h
  IDAction.cpp IDAction.h
  NameAction.cpp NameAction.h
  State.cpp State.h
  Validator.cpp Validator.h
  )

target_link_libraries(grammarLib
  PRIVATE
  )
