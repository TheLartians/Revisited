cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

download_project(
  PROJ catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.7.0
  UPDATE_DISCONNECTED 1
  QUIET
)

# add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})

add_library(Catch2 INTERFACE)

target_include_directories(Catch2
  INTERFACE
    $<BUILD_INTERFACE:${catch2_SOURCE_DIR}/single_include>
    $<INSTALL_INTERFACE:${catch2_SOURCE_DIR}/single_include>
)
