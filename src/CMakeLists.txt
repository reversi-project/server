add_library(
  ${REVERSI_SERVER_LIB}
  game.cc
  game_session.cc
  server.cc
  app.cc
)
target_include_directories(
  ${REVERSI_SERVER_LIB}
  PUBLIC ${REVERSI_SERVER_INCLUDE}
)

find_package(Boost 1.87.0 COMPONENTS system) 
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(
  ${REVERSI_SERVER_LIB}
  PRIVATE ${Boost_LIBRARIES}
)
