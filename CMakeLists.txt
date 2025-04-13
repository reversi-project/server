cmake_minimum_required(VERSION 3.20)
project(reversi-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_CLANG_TIDY "clang-tidy")

set(REVERSI_SERVER_LIB reversi-server-lib)
set(REVERSI_SERVER reversi-server)
set(REVERSI_SERVER_INCLUDE "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(bin)
add_subdirectory(src/reversi/server)

enable_testing()
add_subdirectory(test)
