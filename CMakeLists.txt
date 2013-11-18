# ============================================================================ #
#  Search-A-Gram Instagram Social Network Searcher                             #
#  http://gitlab.eugenecys.com/yjwong/cs3246_assignment3_searchagram           #
# ============================================================================ #
#  Copyright 2013 Wong Yong Jie                                                #
#                                                                              #
#  Licensed under the Apache License, Version 2.0 (the "License");             #
#  you may not use this file except in compliance with the License.            #
#  You may obtain a copy of the License at                                     #
#                                                                              #
#  http://www.apache.org/licenses/LICENSE-2.0                                  #
#                                                                              #
#  Unless required by applicable law or agreed to in writing, software         #
#  distributed under the License is distributed on an "AS IS" BASIS,           #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    #
#  See the License for the specific language governing permissions and         #
#  limitations under the License.                                              #
# ============================================================================ #

# The name of our project is "searchagram". CMakeLists files in this project
# refer to the root source directory of the project as
# ${searchagram_SOURCE_DIR} and to the root binary directory of the project as
# ${searchagram_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (searchagram)

# Set the cmake modules path.
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Require Boost support to build the project.
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME ON)
find_package (Boost 1.54.0 COMPONENTS
  filesystem
  log
  program_options
  system
  thread
  REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})
add_definitions (-DBOOST_REGEX_NO_LIB)

# Require OpenCV to build the project.
if (WIN32)
  set (OpenCV_DIR ${CMAKE_SOURCE_DIR}/external/opencv)
  include_directories (${OpenCV_INCLUDE_DIRS})
endif (WIN32)

find_package (OpenCV REQUIRED core imgproc highgui nonfree features2d flann)

# Build jsoncpp.
option (USE_SYSTEM_JSONCPP "Use the system JsonCpp instead of the included one" OFF)
if (USE_SYSTEM_JSONCPP)
  find_package (JsonCpp REQUIRED)
else ()
  include (BuildJsonCpp)
endif ()
include_directories (${JsonCpp_INCLUDE_DIRS})

# Build libb64.
include (BuildLibb64)
include_directories (${Libb64_INCLUDE_DIRS})

# Build URI.
include (BuildUri)
include_directories (${Uri_INCLUDE_DIRS})

# Build SQLite (required by SOCI).
option (USE_SYSTEM_SQLITE3 "Use the system SQLite instead of the included one" OFF)
if (USE_SYSTEM_SQLITE3)
  find_package (SQLite3 REQUIRED)
else ()
  include (BuildSQLite3)
endif ()

# Build SOCI.
include (BuildSoci)
include_directories (${SOCI_INCLUDE_DIRS})

# Require C++11 support.
find_package (CXXFeatures REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

# Show all warnings from G++.
if (CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif (CMAKE_COMPILER_IS_GNUCXX)

# Enable unit testing support.
enable_testing ()

# Recurse into the "src" subdirectory. This does not actually cause another
# cmake executable to run. The same process will walk through the project's
# entire directory structure.
add_subdirectory (src)

# Add unit tests if requested.
#option (BUILD_TESTS "Builds binaries required for running the test suite." ON)
#if (BUILD_TESTS)
# add_subdirectory (tests)
#endif (BUILD_TESTS)

