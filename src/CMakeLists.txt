add_library(
  ${REVERSI_SERVER_LIB}
  game.cc
  wrapper.cc
  server.cc
)
target_include_directories(
  ${REVERSI_SERVER_LIB}
  PUBLIC ${REVERSI_SERVER_INCLUDE}
)
