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

#include "cosine_hasher.h"

namespace SearchAGram {

  CosineHasher::CosineHasher (int dimensions) :
      generator_ (random_device_), 
      random_projection_ (cv::Mat (1, dimensions, CV_64FC1)) {
    // Generate the random projection vector.
    for (int i = 0; i < dimensions; i++) {
      random_projection_.at<double> (0, i) = distribution_ (generator_);
    }
  }

  int CosineHasher::hash (const cv::Mat& mat) {
    double result = mat.dot (random_projection_);
    return result > 0 ? 1 : 0;
  }

}

/* vim: set ts=2 sw=2 et: */
