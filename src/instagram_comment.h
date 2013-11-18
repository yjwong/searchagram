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

#ifndef SEARCHAGRAM_INSTAGRAM_COMMENT_H_
#define SEARCHAGRAM_INSTAGRAM_COMMENT_H_

#include <string>

#include "instagram_user.h"

namespace SearchAGram {
  
  /**
   * Represents an Instagram Comment.
   *
   * The list of fields is described at:
   * http://instagram.com/developer/endpoints/comments/
   */
  struct InstagramComment {
    int created_time;
    std::string text;
    InstagramUser from;
    std::string id;
  };

}

#endif /* SEARCHAGRAM_INSTAGRAM_COMMENT_H_ */

/* vim: set ts=2 sw=2 et: */
