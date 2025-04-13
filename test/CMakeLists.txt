include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.16.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(REVERSI_SERVER_TESTS reversi-server-tests)
add_executable(
  ${REVERSI_SERVER_TESTS}
  main.cc
  server_test.cc
)
target_link_libraries(
  ${REVERSI_SERVER_TESTS}
  ${REVERSI_SERVER_LIB}
  GTest::gtest_main
)
target_include_directories(
  ${REVERSI_SERVER_TESTS}
  PRIVATE ${REVERSI_SERVER_INCLUDE}
)

include(GoogleTest)
gtest_discover_tests(${REVERSI_SERVER_TESTS})
