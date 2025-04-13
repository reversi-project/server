add_library(
  ${REVERSI_SERVER_LIB}
  app.cc
)

include(FetchContent)
FetchContent_Declare(
  reversi-contract
  GIT_REPOSITORY https://github.com/reversi-project/contract.git
  GIT_TAG master
)
FetchContent_MakeAvailable(reversi-contract)

find_package(Boost 1.87.0 COMPONENTS system) 
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(
  ${REVERSI_SERVER_LIB}
  ${Boost_LIBRARIES}
  reversi-contract-lib
)
target_include_directories(
  ${REVERSI_SERVER_LIB} PRIVATE
  ${REVERSI_SERVER_INCLUDE}
  ${reversi-contract_SOURCE_DIR}/include
)
