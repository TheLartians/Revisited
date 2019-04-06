cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# ---- Project ----

project(Tests CXX)

# ---- CXX Flags ----

if(NOT CMAKE_CXX_STANDARD GREATER 17)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# ---- Requires ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(LarsVisitor REQUIRED)
endif()

add_subdirectory(../dependencies/catch2 ${PROJECT_BINARY_DIR}/catch2)

# ---- Create binary ----

file(GLOB tests_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(LarsVisitorTests ${tests_sources})

target_link_libraries(LarsVisitorTests Catch2)
target_link_libraries(LarsVisitorTests LarsVisitor)

# ---- Add tests ----

ENABLE_TESTING() 
ADD_TEST(LarsVisitorTests LarsVisitorTests)

