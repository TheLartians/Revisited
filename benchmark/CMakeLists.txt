cmake_minimum_required (VERSION 3.14)

# ---- create project ----

project(RevisitedBenchmark 
  LANGUAGES CXX
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CPM.cmake)

CPMAddPackage(
  NAME Revisited
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..
)

CPMAddPackage(
  NAME googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  VERSION 1.4.1
  OPTIONS
   "BENCHMARK_ENABLE_TESTING Off"
   "BENCHMARK_USE_LIBCXX ON"
)

# fix google benchmark
set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)        

add_executable(RevisitedBenchmark "benchmark.cpp")
target_link_libraries(RevisitedBenchmark Revisited benchmark)
set_target_properties(RevisitedBenchmark PROPERTIES CXX_STANDARD 17)        
