
cmake_minimum_required (VERSION 3.0)

# ---- create project ----

project(LarsVisitor 
  VERSION 0.3
  LANGUAGES CXX
)

# ---- Configuration variables ----

option(ENABLE_LARS_VISITOR_TESTS "Enable tests" OFF)

# ---- Include guards ----

if(TARGET LarsVisitor)
  return()
endif()

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
      message(FATAL_ERROR "Source and build directories cannot be the same.")
endif()

# ---- dependencies ----

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ctti)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/LHC)

# ---- Header target ----

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/include/lars/*.h")
add_library(LarsVisitor-headers EXCLUDE_FROM_ALL ${headers})
SET_TARGET_PROPERTIES(LarsVisitor-headers PROPERTIES LINKER_LANGUAGE CXX)

# ---- includes ----

add_library(LarsVisitor INTERFACE)

target_link_libraries(LarsVisitor INTERFACE LHC ctti)

target_include_directories(LarsVisitor
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ---- Install ----

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/LarsVisitorConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  TARGETS LarsVisitor
  EXPORT LarsVisitorTargets
  LIBRARY DESTINATION lib COMPONENT Runtime
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  PUBLIC_HEADER DESTINATION include COMPONENT Development
  BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/LarsVisitorConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/LarsVisitorConfig.cmake"
  INSTALL_DESTINATION lib/cmake/LarsVisitor
)

install(
  EXPORT LarsVisitorTargets 
  DESTINATION lib/cmake/LarsVisitor
)

install(
  FILES 
    "${PROJECT_BINARY_DIR}/LarsVisitorConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/LarsVisitorConfig.cmake"
  DESTINATION 
    lib/cmake/LarsVisitor
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include
)

# ---- Test ----

if(${ENABLE_LARS_VISITOR_TESTS})
  ENABLE_TESTING()  
  add_subdirectory(tests)  
endif()
