cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if(TARGET ctti)
  return()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DownloadProject.cmake)

download_project(
  PROJ ctti
  GIT_REPOSITORY https://github.com/Manu343726/ctti.git
  GIT_TAG d7e9828
  UPDATE_DISCONNECTED 1
  QUIET
)

add_library(ctti INTERFACE)

target_include_directories(ctti
  INTERFACE
  $<BUILD_INTERFACE:${ctti_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
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

install(
  EXPORT cttiTargets 
  DESTINATION lib/cmake/ctti
)

install(
  DIRECTORY ${ctti_SOURCE_DIR}/include/
  DESTINATION include
)

# Don't add the project, as it defines no install target
# add_subdirectory(${ctti_SOURCE_DIR} ${ctti_BINARY_DIR})

