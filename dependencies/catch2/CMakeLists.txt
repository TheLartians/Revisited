cmake_minimum_required(VERSION 3.13)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

download_project(
  PROJ catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.7.0
  UPDATE_DISCONNECTED 1
  QUIET
)

add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})


