cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

project(LarsVisitorTests CXX)

# ---- CXX Flags ----

if(NOT CMAKE_CXX_STANDARD GREATER 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror")

# ---- Requires ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(LarsVisitor REQUIRED)
endif()

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.5.0
)

FetchContent_MakeAvailable(Catch2)

# ---- Create binary ----

file(GLOB tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(LarsVisitorTests ${tests_sources})

target_link_libraries(LarsVisitorTests LarsVisitor Catch2)

# ---- Add tests ----

ENABLE_TESTING() 
ADD_TEST(LarsVisitorTests LarsVisitorTests)

# ---- code coverage ----

if (${ENABLE_TEST_COVERAGE})
  set_target_properties(LarsVisitorTests PROPERTIES CXX_STANDARD 17 COMPILE_FLAGS "-O0 -g -fprofile-arcs -ftest-coverage --coverage")
  target_link_options(LarsVisitorTests PUBLIC "--coverage")
endif()
