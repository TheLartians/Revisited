cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(TARGET dependency-LHC)
  return()
endif()

find_package(LHC 0.1 QUIET)

if(${LHC_FOUND})
  set_target_properties(LHC 
    PROPERTIES
      IMPORTED_GLOBAL True
  )
else()
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

  download_project(
    PROJ LHC
    GIT_REPOSITORY https://github.com/TheLartians/LHC.git
    GIT_TAG fc7f199
    UPDATE_DISCONNECTED 1
    QUIET
  )

  add_subdirectory(${LHC_SOURCE_DIR} ${LHC_BINARY_DIR})
endif()

add_library(dependency-LHC ALIAS LHC)

