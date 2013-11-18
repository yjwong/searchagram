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

#ifndef SEARCHAGRAM_ANALYSIS_COLOR_COHERENCE_VECTOR_H_
#define SEARCHAGRAM_ANALYSIS_COLOR_COHERENCE_VECTOR_H_

#include <string>
#include <vector>
#include <map>
#include "opencv2/opencv.hpp"

namespace SearchAGram {
  class ColorCoherenceVector {
  public:
    static const int DEFAULT_DISCRETIZE_BPC;
    static const std::vector<cv::Point>::size_type DEFAULT_TAU;

    ColorCoherenceVector (const std::string& filename);
    ColorCoherenceVector (cv::Mat mat);

    void setFilterEngine (cv::Ptr<cv::FilterEngine> filter_engine);
    cv::Ptr<cv::FilterEngine> getFilterEngine ();

    void setDiscretizeBPC (int bpc);
    int getDiscretizeBPC ();

    void setTau (std::vector<cv::Point>::size_type tau);
    std::vector<cv::Point>::size_type getTau ();

    void compute ();

    cv::Mat getCCV ();
    cv::Mat getImage ();
    cv::Mat getImage (int width, int height);

  private:
    ColorCoherenceVector ();

    // Contains the image to extract the CCV from
    cv::Mat image_;
    cv::Mat computed_image_;

    // The filter engine to use in the pre-processing stage
    cv::Ptr<cv::FilterEngine> filter_engine_;

    // Discretization parameters
    int discretize_bpc_;

    // Tau parameter
    std::vector<cv::Point>::size_type tau_;

    // State information
    bool computed_;
    cv::Mat visited_pixels_;
    std::vector<std::vector<cv::Point>> regions_;

    void computedCheck_ ();
    void applyFilterEngine_ ();
    void discretizeColorSpace_ (int bpc);
    std::vector<cv::Point> findRegionMatchingPixel_ (int row, int col);
  };
}

#endif /* SEARCHAGRAM_ANALYSIS_COLOR_COHERENCE_VECTOR_H_ */

/* vim: set ts=2 sw=2 et: */
