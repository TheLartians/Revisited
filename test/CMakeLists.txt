cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(RevisitedTests
  LANGUAGES CXX
)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY onqtam/doctest
  GIT_TAG 2.3.7
)

if (TEST_INSTALLED_VERSION)
  find_package(Revisited REQUIRED)
else()
  CPMAddPackage(
    NAME Revisited
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
  )
endif()

CPMAddPackage(
  NAME Format.cmake
  GITHUB_REPOSITORY TheLartians/Format.cmake
  VERSION 1.0
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(RevisitedTests ${sources})
target_link_libraries(RevisitedTests doctest Revisited)

set_target_properties(RevisitedTests PROPERTIES CXX_STANDARD 17)

# enable compiler warnings
if (NOT TEST_INSTALLED_VERSION)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Revisited INTERFACE -Wall -pedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(Revisited INTERFACE /W4 /WX)
  endif()
endif()

# ---- Add RevisitedTests ----

ENABLE_TESTING() 

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(RevisitedTests)

# ---- code coverage ----

if (ENABLE_TEST_COVERAGE)
  target_compile_options(RevisitedTests PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(RevisitedTests PUBLIC -fprofile-arcs -ftest-coverage)
endif()
