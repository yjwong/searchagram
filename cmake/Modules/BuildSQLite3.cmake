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

include (ExternalProject)
ExternalProject_Add (sqlite
    PREFIX ${CMAKE_BINARY_DIR}/3rdparty/sqlite
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/sqlite
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-tcl
    BUILD_COMMAND make
    INSTALL_COMMAND make install)

ExternalProject_Add_Step (sqlite create_fossil_manifest
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/BuildSQLite3/create-fossil-manifest
    COMMENT "Creating Fossil manifest..."
    DEPENDEES configure
    DEPENDERS build
    WORKING_DIRECTORY <SOURCE_DIR>)

set (SQLite3_ROOT_DIR ${CMAKE_BINARY_DIR}/3rdparty/sqlite)
set (SQLite3_LIBRARIES
    ${SQLite3_ROOT_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX})
set (SQLite3_INCLUDE_DIRS ${SQLite3_ROOT_DIR}/include)

