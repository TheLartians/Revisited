cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(TARGET ctti)
  return()
endif()

project(ctti 
  VERSION 0.0.1
  LANGUAGES CXX
)

FetchContent_Declare(
  ctti
  GIT_REPOSITORY https://github.com/Manu343726/ctti.git
  GIT_TAG d7e9828
  UPDATE_DISCONNECTED 1
  QUIET
)

FetchContent_GetProperties(ctti)
if(NOT ctti_POPULATED)
  FetchContent_Populate(ctti)
endif()

# Don't add the project, as it defines no install target
# add_subdirectory(${ctti_SOURCE_DIR} ${ctti_BINARY_DIR})

add_library(ctti INTERFACE)

target_include_directories(ctti
  INTERFACE
  $<BUILD_INTERFACE:${ctti_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/cttiConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  TARGETS ctti
  EXPORT cttiTargets
  LIBRARY DESTINATION lib COMPONENT Runtime
  ARCHIVE DESTINATION lib COMPONENT Development
  RUNTIME DESTINATION bin COMPONENT Runtime
  PUBLIC_HEADER DESTINATION include COMPONENT Development
  BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cttiConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/cttiConfig.cmake"
  INSTALL_DESTINATION lib/cmake/ctti
)

install(
  EXPORT cttiTargets 
  DESTINATION lib/cmake/ctti
)

install(
  FILES 
    "${PROJECT_BINARY_DIR}/cttiConfigVersion.cmake"
    "${PROJECT_BINARY_DIR}/cttiConfig.cmake"
  DESTINATION 
  lib/cmake/ctti
)

install(
  DIRECTORY ${ctti_SOURCE_DIR}/include/
  DESTINATION include
)
