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

#include "opencv2/nonfree/features2d.hpp"
#include "surf_vector.h"

namespace SearchAGram {
  SurfVector::SurfVector (const std::string& filename) :
      min_hessian_ (400) {
    image_ = cv::imread (filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (!image_.data) {
      throw std::runtime_error ("Unable to open file to construct histogram");
    }
  }

  SurfVector::SurfVector (const cv::Mat& mat) :
      min_hessian_ (400) {
    image_ = mat.clone ();
    cv::cvtColor (image_, image_, CV_BGR2GRAY);
  }

  void SurfVector::setMinHessian (int min_hessian) {
    min_hessian_ = min_hessian;
  }

  int SurfVector::getMinHessian () {
    return min_hessian_;
  }

  std::vector<cv::KeyPoint> SurfVector::getKeyPoints () {
    cv::SurfFeatureDetector detector (min_hessian_);
    std::vector<cv::KeyPoint> keypoints;

    // Detect the keypoints.
    detector.detect (image_, keypoints);

    return keypoints;
  }

  cv::Mat SurfVector::detect () {
    std::vector<cv::KeyPoint> keypoints = getKeyPoints ();

    // Calculate the feature vectors.
    cv::SurfDescriptorExtractor extractor;
    cv::Mat descriptors;
    extractor.compute (image_, keypoints, descriptors);

    return descriptors;
  }

  cv::Mat SurfVector::getImage () {
    cv::Mat image;
    std::vector<cv::KeyPoint> keypoints = getKeyPoints ();
    cv::drawKeypoints (image_, keypoints, image, cv::Scalar::all (-1),
      cv::DrawMatchesFlags::DEFAULT);

    return image;
  }
}

/* vim: set ts=2 sw=2 et: */
