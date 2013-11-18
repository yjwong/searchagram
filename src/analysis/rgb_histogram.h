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

#ifndef SEARCHAGRAM_ANALYSIS_RGB_HISTOGRAM_H_
#define SEARCHAGRAM_ANALYSIS_RGB_HISTOGRAM_H_

#include <string>
#include "opencv2/opencv.hpp"

namespace SearchAGram {
  class RGBHistogram {
  public:
    enum Channel {
      CHANNEL_BLUE = 0,
      CHANNEL_GREEN = 1,
      CHANNEL_RED = 2
    };

    RGBHistogram (const std::string& filename);
    RGBHistogram (cv::Mat mat);

    cv::Mat getImage ();
    cv::Mat getImage (int width, int height);

    cv::Mat getHistogram (Channel channel);

    double getBinValue (Channel channel, int bin);
    int getBinCount ();

  private:
    RGBHistogram () { };

    cv::Mat red_histogram_;
    cv::Mat green_histogram_;
    cv::Mat blue_histogram_;

    void _computeHistogram (cv::Mat image);
    cv::Mat _computeHistogramForChannel (cv::Mat image, Channel channel);
  };
}

#endif /* SEARCHAGRAM_ANALYSIS_RGB_HISTOGRAM_H_ */

/* vim: set ts=2 sw=2 et: */
