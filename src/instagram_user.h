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

#ifndef SEARCHAGRAM_INSTAGRAM_USER_H_
#define SEARCHAGRAM_INSTAGRAM_USER_H_

#include "soci.h"
#include <string>

namespace SearchAGram {
  
  /**
   * Represents an Instagram User.
   *
   * The list of fields is described at:
   * http://instagram.com/developer/endpoints/users/
   */
  struct InstagramUser {
    std::string id;
    std::string username;
    std::string full_name;
    std::string profile_picture;
    std::string bio;
    std::string website;
  };
  
}

namespace soci {
  template<>
  struct type_conversion<SearchAGram::InstagramUser> {
    typedef values base_type;

    static void from_base (values const& v, indicator,
        SearchAGram::InstagramUser u) {
      u.id = v.get<std::string> ("id");
      u.username = v.get<std::string> ("username");
      u.full_name = v.get<std::string> ("full_name");
      u.profile_picture = v.get<std::string> ("profile_picture");
      u.bio = v.get<std::string> ("bio");
      u.website = v.get<std::string> ("website");
    }

    static void to_base (const SearchAGram::InstagramUser& u, values& v,
        indicator& ind) {
      v.set ("id", u.id);
      v.set ("username", u.username);
      v.set ("full_name", u.full_name);
      v.set ("profile_picture", u.profile_picture);
      v.set ("bio", u.bio);
      v.set ("website", u.website);

      ind = i_ok;
    }
  };
}

#endif /* SEARCHAGRAM_INSTAGRAM_USER_H_ */

/* vim: set ts=2 sw=2 et: */
