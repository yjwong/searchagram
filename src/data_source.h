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

#ifndef SEARCHAGRAM_DATA_SOURCE_H_
#define SEARCHAGRAM_DATA_SOURCE_H_

#include "opencv2/opencv.hpp"
#include "instagram_image.h"

namespace SearchAGram {
  
  class DataSource {
  public:
    bool operator>> (InstagramImage& out);

    virtual InstagramImage getNext () = 0;
    virtual bool hasMore () = 0;
    virtual cv::Mat getMat (const InstagramImage& image) = 0;
    virtual std::string getMatLocation (const InstagramImage& image) = 0;
  };

}

#endif /* SEARCHAGRAM_DATA_SOURCE_H_ */

/* vim: set ts=2 sw=2 et: */
