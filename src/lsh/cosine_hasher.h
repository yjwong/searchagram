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

#ifndef SEARCHAGRAM_COSINE_HASHER_H_
#define SEARCHAGRAM_COSINE_HASHER_H_

#include <random>
#include "hasher.h"

namespace SearchAGram {

  class CosineHasher : public Hasher {
  public:
    CosineHasher (int dimensions);
    virtual int hash (const cv::Mat& mat);

  private:
    std::normal_distribution<> distribution_;
    std::random_device random_device_;
    std::mt19937 generator_;

    cv::Mat random_projection_;
  };

}

#endif /* SEARCHAGRAM_COSINE_HASHER_H_ */

/* vim: set ts=2 sw=2 et: */
