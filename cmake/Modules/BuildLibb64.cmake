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

set (Libb64_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/libb64)

add_library (b64
  ${Libb64_ROOT}/src/cencode.c
  ${Libb64_ROOT}/src/cdecode.c)

set_target_properties (b64
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/3rdparty/libb64/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/3rdparty/libb64/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/3rdparty/libb64/bin")

set (Libb64_INCLUDE_DIRS ${Libb64_ROOT}/include)
set (Libb64_LIBRARIES b64)

