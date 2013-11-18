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

#ifndef SEARCHAGRAM_ANALYSIS_EDGE_DIRECTION_HISTOGRAM_H_
#define SEARCHAGRAM_ANALYSIS_EDGE_DIRECTION_HISTOGRAM_H_

#include <string>
#include "opencv2/opencv.hpp"

namespace SearchAGram {
  class EdgeDirectionHistogram {
  public:
    EdgeDirectionHistogram (const std::string& filename);
    EdgeDirectionHistogram (cv::Mat mat);

    void compute ();

    int getQuantizationLevels ();
    void setQuantizationLevels (int levels);

    int getBinValue (int bin);
    int getBinCount ();

    cv::Mat getHistogram ();

    cv::Mat getImage ();
    cv::Mat getImage (int height, int width);

  private:
    cv::Mat image_;
    cv::Mat computed_image_;

    // Normalized image gradients.
    cv::Mat gradient_x_;
    cv::Mat gradient_y_;
    cv::Mat gradient_;
    cv::Mat direction_;
    cv::Mat direction_histogram_;

    // Parameters.
    int quantization_levels_;

    EdgeDirectionHistogram () { };
    void convertToLuminance_ ();
    void applySobel_ ();
    void computeDirection_ ();
    void quantizeDirection_ ();
  };
}

#endif /* SEARCHAGRAM_ANALYSIS_EDGE_DIRECTION_HISTOGRAM_H_ */

/* vim: set ts=2 sw=2 et: */
