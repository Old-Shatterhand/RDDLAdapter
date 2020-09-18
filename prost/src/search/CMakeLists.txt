cmake_minimum_required(VERSION 2.8.0)

if(NOT PROST_MAIN_CMAKELISTS_READ)
  message(
    FATAL_ERROR
    "Run cmake on the CMakeLists.txt in the 'src' directory, "
    "not the one in 'src/search'. Please delete CMakeCache.txt "
    "and CMakeFiles/ from the current directory and restart cmake.")
endif()

project(prost)
prost_set_compiler_flags()
prost_set_linker_flags_search()

find_package(BDD REQUIRED)
if(NOT BDD_FOUND)
  message(FATAL_ERROR "BuDDy lib was not found. Please install it using "
  "sudo apt-get install libbdd-dev "
  " and re-run the build process.")
endif()

include_directories("logical_expressions_includes")
include_directories("utils")

set(SEARCH_SOURCES
  adapter
  evaluatables
  ipc_client
  logical_expressions
  parser
  probability_distribution
  prost_planner
  search_engine
  states
  utils/base64
  utils/logger
  utils/math_utils
  utils/pstream
  utils/random
  utils/stopwatch
  utils/string_utils
  utils/strxml
  utils/system_utils
 )

set(SEARCH_TESTS
  ../doctest/doctest
  tests/evaluate_test
  tests/probability_distribution_test
)

# Linking BDD library required for reward lock detection
set(GPP_LINK_FLAGS "${GPP_LINK_FLAGS} -lbdd")

# Setting flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GPP_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GPP_LINK_FLAGS}")

# disable doctest in release build
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDOCTEST_CONFIG_DISABLE")

# only enable unit tests in debug build
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_executable(search ${SEARCH_SOURCES} ${SEARCH_TESTS} main)
ELSE()
    add_executable(search ${SEARCH_SOURCES} main)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

# Linking BDD
target_link_libraries(search bdd)
