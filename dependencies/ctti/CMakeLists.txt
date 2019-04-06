cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

download_project(
  PROJ ctti
  GIT_REPOSITORY https://github.com/Manu343726/ctti.git
  GIT_TAG d7e9828
  UPDATE_DISCONNECTED 1
  QUIET
)

add_subdirectory(${ctti_SOURCE_DIR} ${ctti_BINARY_DIR})

