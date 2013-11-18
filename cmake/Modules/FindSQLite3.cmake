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

# Use pkg-config to provide some hints.
find_package (PkgConfig)
pkg_check_modules (PC_SQLite3_QUIET sqlite3)
set (SQLite3_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

# Look for the headers.
find_path (SQLite3_INCLUDE_DIR sqlite3.h
    HINTS ${PC_SQLite3_INCLUDEDIR} ${PC_SQLite3_INCLUDE_DIRS})

# Look for the libraries.
find_library (SQLite3_LIBRARY NAMES sqlite3 libsqlite3
    HINTS ${PC_SQLite3_LIBDIR} ${PC_SQLite3_LIBRARY_DIRS})

set (SQLite3_LIBRARIES ${SQLite3_LIBRARY})
set (SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (SQLite3 DEFAULT_MSG
    SQLite3_LIBRARY SQLite3_INCLUDE_DIR)

mark_as_advanced (SQLite3_INCLUDE_DIR SQLite3_LIBRARY)

