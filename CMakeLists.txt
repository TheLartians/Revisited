
cmake_minimum_required (VERSION 2.6)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
      message(FATAL_ERROR "Source and build directories cannot be the same.")
endif()

# ---- create project ----

project(LarsVisitor)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(PROJECT_INCUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)


# ---- add sources ----

file (GLOB sources ${PROJECT_SOURCE_DIR}/*.cpp)
file (GLOB headers ${PROJECT_INCUDE_DIR}/lars/*.h)

# ---- enable c++14 ----

if(NOT CMAKE_CXX_STANDARD GREATER 14)
  set (CMAKE_CXX_STANDARD 14)
endif()

# ---- create module ----

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")


# ---- includes ----

IF(LARS_LIBRARY)
  SET(lars_headers ${lars_headers} ${headers} PARENT_SCOPE)  
  SET(lars_include_dirs ${lars_include_dirs} "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
  SET(lars_libraries ${lars_libraries} tape ducktape PARENT_SCOPE)
  SET(Lars_INCLUDE_DIRS ${lars_include_dirs})
ELSE()
  message("Visitor has to be built as a lars library module")
ENDIF()

# ---- examples ----

IF(LARS_BUILD_EXAMPLES)
  file( GLOB example_sources ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp )
  foreach( examplesourcefile ${example_sources} )
    get_filename_component(filename ${examplesourcefile} NAME)
    string( REPLACE ".cpp" "" examplename ${filename} )
    set(examplename lars_${examplename})
    add_executable( ${examplename} ${examplesourcefile})
    include_directories(${Lars_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include")
  endforeach()
ENDIF()


