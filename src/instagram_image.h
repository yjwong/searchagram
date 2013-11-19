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

#ifndef SEARCHAGRAM_INSTAGRAM_IMAGE_H_
#define SEARCHAGRAM_INSTAGRAM_IMAGE_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "instagram_user.h"
#include "instagram_comment.h"
#include "instagram_like.h"

namespace SearchAGram {

  /**
   * Represents an Instagram Image.
   *
   * The list of fields is described at:
   * http://instagram.com/developer/endpoints/media/
   */
  struct InstagramImage {
    /* Represents an Instagram User in a photo */
    struct UserInPhoto {
      InstagramUser user;
      struct position {
        double x;
        double y;
      };
    };

    /* Represents an Instagram Source Image */
    struct SourceImage {
      std::string url;
      int width;
      int height;
    };

    /* Represents an Instagram caption */
    /*
    struct Caption {
      std::string id;
      std::string text;
      int created_time;
    };
    */

    std::string id;
    std::string type;
    std::vector<UserInPhoto> users_in_photo;
    std::string filter;
    std::vector<std::string> tags;
    std::vector<InstagramComment> comments;
    int comments_count;
    //Caption caption;
    std::string caption;
    std::vector<InstagramLike> likes;
    int likes_count;
    std::string link;
    InstagramUser user;
    int created_time;
    std::unordered_map<std::string, SourceImage> images;
    bool user_has_liked;
  };

}

namespace soci {
  template<>
  struct type_conversion<SearchAGram::InstagramImage> {
    typedef values base_type;

    static void from_base (values const& v, indicator,
        SearchAGram::InstagramImage i) {
      i.id = v.get<std::string> ("id");
      i.type = v.get<std::string> ("type");
      i.filter = v.get<std::string> ("filter");
      i.comments_count = v.get<int> ("comments_count");
      i.caption = v.get<std::string> ("caption");
      i.likes_count = v.get<int> ("likes_count");
      i.link = v.get<std::string> ("link");
      i.user.id = v.get<std::string> ("user_id");
      i.created_time = v.get<int> ("created_time");
      i.user_has_liked = v.get<bool> ("user_has_liked");
    }

    static void to_base (const SearchAGram::InstagramImage& i, values& v,
        indicator& ind) {
      v.set ("id", i.id);
      v.set ("type", i.type);
      v.set ("filter", i.filter);
      v.set ("comments_count", i.comments_count);
      v.set ("caption", i.caption);
      v.set ("likes_count", i.likes_count);
      v.set ("link", i.link);
      v.set ("user_id", i.user.id);
      v.set ("created_time", i.created_time);
      v.set ("user_has_liked", i.user_has_liked ? 1 : 0);

      ind = i_ok;
    }
  };

  template<>
  struct type_conversion<SearchAGram::InstagramImage::SourceImage> {
    typedef values base_type;

    static void from_base (values const& v, indicator,
        SearchAGram::InstagramImage::SourceImage s) {
      s.url = v.get<std::string> ("url");
      s.width = v.get<int> ("width");
      s.height = v.get<int> ("height");
    }

    static void to_base (const SearchAGram::InstagramImage::SourceImage& s,
        values& v, indicator& ind) {
      v.set ("url", s.url);
      v.set ("width", s.width);
      v.set ("height", s.height);

      ind = i_ok;
    }
  };
}

#endif /* SEARCHAGRAM_INSTAGRAM_IMAGE_H_ */

/* vim: set ts=2 sw=2 et: */
