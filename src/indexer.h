/* ========================================================================
 * Search-A-Gram Instagram Social Network Searcher
 * http://gitlab.eugenecys.com/yjwong/cs3246_assignment3_searchagram
 * ========================================================================
 * Copyright 2013 Wong Yong Jie
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ======================================================================== */

#ifndef SEARCHAGRAM_INDEXER_H_
#define SEARCHAGRAM_INDEXER_H_

#include <string>

namespace SearchAGram {

  class Indexer {
  public:
    static int main (int argc, char* argv[]);

  private:
    static std::string config_;

    static const int EXIT_OK = 0;
    static const int EXIT_PROGRAM_OPTIONS_ERROR = 1;
    static const int EXIT_HELP_CALLED = 2;

    static const int EXIT_CONFIG_NOT_EXIST = 3;
    static const int EXIT_CONFIG_NOT_REGULAR_FILE = 4;
    static const int EXIT_CONFIG_NOT_READABLE = 5;
    static const int EXIT_CONFIG_PARSE_ERROR = 6;

    static const int EXIT_DATA_SOURCE_ERROR = 7;

    static int parseArguments_ (int argc, char* argv[]);
    static int checkConfig_ ();

    static void buildIndex_ ();
    static void buildFlannIndex_ ();
  };

}

#endif /* SEARCHAGRAM_INDEXER_H_ */

/* vim: set ts=2 sw=2 et: */
