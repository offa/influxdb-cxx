####################################
# General project definition
####################################

cmake_minimum_required(VERSION 3.12.0)

# disable testsuite when included
# using add_subdirectory
if(DEFINED PROJECT_NAME)
  set(INCLUDED_AS_SUBPROJECT ON)
  set(INFLUXCXX_TESTING OFF CACHE BOOL "testing not available in sub-project")
  set(INFLUXCXX_SYSTEMTEST OFF CACHE BOOL "system testing not available in sub-project")
  set(INFLUXCXX_COVERAGE OFF CACHE BOOL "coverage not available in sub-project")
endif()

option(BUILD_SHARED_LIBS "Build shared versions of libraries" ON)
option(INFLUXCXX_WITH_BOOST "Build with Boost support enabled" ON)
option(INFLUXCXX_TESTING "Enable testing for this component" ON)
option(INFLUXCXX_SYSTEMTEST "Enable system tests" ON)
option(INFLUXCXX_COVERAGE "Enable Coverage" OFF)

# Define project
project(influxdb-cxx
  VERSION 0.7.0
  DESCRIPTION "InfluxDB C++ client library"
  LANGUAGES CXX
)

message(STATUS "~~~ ${PROJECT_NAME} v${PROJECT_VERSION} ~~~")


# Add compiler flags for warnings
if(NOT MSVC AND NOT INCLUDED_AS_SUBPROJECT)
    add_compile_options(-Wall
      -Wextra
      -pedantic
      -pedantic-errors
      -Werror
      -Wshadow
      -Wold-style-cast
      -Wnull-dereference
      -Wnon-virtual-dtor
      -Woverloaded-virtual
      )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# We explicitly export the public interface
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
include(GenerateExportHeader)

# Set fPIC for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set the default build type to "RelWithDebInfo"
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo"
    CACHE
    STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
endif()

message(STATUS "Build Type : ${CMAKE_BUILD_TYPE}")
message(STATUS "Boost support : ${INFLUXCXX_WITH_BOOST}")
message(STATUS "Unit Tests : ${INFLUXCXX_TESTING}")
message(STATUS "System Tests : ${INFLUXCXX_TESTING}")


# Add coverage flags
if(INFLUXCXX_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

####################################
# Dependencies
####################################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(cpr REQUIRED)

if (INFLUXCXX_WITH_BOOST)
    # Fixes warning when using boost from brew
    set(Boost_USE_MULTITHREADED TRUE)

    find_package(Boost REQUIRED COMPONENTS system)
endif()

add_subdirectory(3rd-party)

####################################
# Library
####################################

# Create library
# note: BUILD_SHARED_LIBS specifies if static or shared
#       as boost is build without -fPIC, we cannot
#       statically link against it when building
#       influxdb as shared object
add_subdirectory("src")


####################################
# Tests
####################################

if (INFLUXCXX_TESTING)
  enable_testing()
  add_subdirectory("test")
endif()


####################################
# Install
####################################

include(GNUInstallDirs)

# Build targets with install rpath on Mac to dramatically speed up installation
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
unset(isSystemDir)

# Install library
install(TARGETS InfluxDB
  EXPORT InfluxDBTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Create version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/InfluxDBConfigVersion.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES ${PROJECT_BINARY_DIR}/src/influxdb_export.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Export targets
install(EXPORT InfluxDBTargets
  FILE
    InfluxDBTargets.cmake
  NAMESPACE
    InfluxData::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/InfluxDB
)

# Configure and install Config files
configure_package_config_file(
  cmake/InfluxDBConfig.cmake.in cmake/InfluxDBConfig.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/InfluxDB"
  PATH_VARS CMAKE_INSTALL_PREFIX
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/InfluxDBConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/InfluxDBConfigVersion.cmake"
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/InfluxDB
)
