add_executable(${REVERSI_SERVER} main.cc)

target_link_libraries(
  ${REVERSI_SERVER}
  PRIVATE ${REVERSI_SERVER_LIB}
)
target_include_directories(
  ${REVERSI_SERVER}
  PUBLIC ${REVERSI_SERVER_INCLUDE}
)
