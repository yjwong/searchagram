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

set (soci_CONFIGURE_COMMAND ${CMAKE_COMMAND})
set (soci_CONFIGURE_COMMAND ${soci_CONFIGURE_COMMAND}
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSQLITE_ROOT_DIR=${SQLite3_ROOT_DIR}
    -DSOCI_TESTS=OFF
    <SOURCE_DIR>/src)

include (ExternalProject)
ExternalProject_Add (soci
    PREFIX ${CMAKE_BINARY_DIR}/3rdparty/soci
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/soci
    CONFIGURE_COMMAND ${soci_CONFIGURE_COMMAND}
    BUILD_COMMAND make
    INSTALL_COMMAND make install)
add_dependencies (soci sqlite)

# Set the root path.
set (SOCI_ROOT_DIR ${CMAKE_BINARY_DIR}/3rdparty/soci)

# Move the lib64 to lib on 64-bit systems
ExternalProject_Add_Step (soci move_lib64
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOCI_ROOT_DIR}/lib64 ${SOCI_ROOT_DIR}/lib
    DEPENDEES install)

# Figure out the library paths.
set (SOCI_LIBRARIES
    ${SOCI_ROOT_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}soci_core${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${SOCI_ROOT_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}soci_empty${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${SOCI_ROOT_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}soci_sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX})

# Set the include path.
set (SOCI_INCLUDE_DIRS
    ${SOCI_ROOT_DIR}/include
    ${CMAKE_SOURCE_DIR}/3rdparty/soci/src/core
    ${CMAKE_SOURCE_DIR}/3rdparty/soci/src/backends/sqlite3)

