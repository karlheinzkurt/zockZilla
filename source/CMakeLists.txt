cmake_minimum_required(VERSION 3.1.2)
project(gsc)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  SET(CMAKE_BUILD_TYPE "Debug")
  add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
  link_libraries(gcov)
  MESSAGE(STATUS "Configure build for coverage analysis")
ENDIF()

add_compile_options(--std=c++14)
add_compile_options(-Wall)
add_compile_options(-Werror)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# set(Boost_USE_STATIC_LIBS ON)
# find_package(Boost 1.72.0 REQUIRED program_options)

find_library(Log4cxx REQUIRED log4cxx) # There is no conan package
SET(Log4cxx_LIBRARIES log4cxx) # There is no findpackage

add_subdirectory(gsclib)
add_subdirectory(gsc)

SET(CPACK_PACKAGE_NAME "gsc")
SET(CPACK_PACKAGE_DESCRIPTION "Tool to monitor and control gaming sessions")
SET(CPACK_PACKAGE_CONTACT "Sascha Lumma")
SET(CPACK_PACKAGE_VERSION "0.1.0")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_FILE_NAME "gsc-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
SET(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "utils")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

# systemctl --system daemon-reload
# systemctl enable gsc
# systemctl start gsc
 
INCLUDE(CPack)

