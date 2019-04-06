cmake_minimum_required(VERSION 3.13)

find_package(LHC 0.1 QUIET)
if(${LHC_FOUND})
  return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

download_project(
  PROJ LHC
  GIT_REPOSITORY https://github.com/TheLartians/LHC.git
  GIT_TAG fc7f199
  UPDATE_DISCONNECTED 1
  QUIET
)

add_subdirectory(${LHC_SOURCE_DIR} ${LHC_BINARY_DIR})

