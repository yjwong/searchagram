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

#include <vector>
#include <cmath>

#include "edge_direction_histogram.h"

namespace SearchAGram {
  EdgeDirectionHistogram::EdgeDirectionHistogram (const std::string& filename) :
      quantization_levels_ (8) {
    image_ = cv::imread (filename, 1);
    if (!image_.data) {
      throw std::runtime_error ("Unable to open file to construct histogram");
    }
  }

  EdgeDirectionHistogram::EdgeDirectionHistogram (cv::Mat mat) :
      quantization_levels_ (8) {
    image_ = mat;
  }

  void EdgeDirectionHistogram::compute () {
    convertToLuminance_ ();
    applySobel_ ();
    computeDirection_ ();
    quantizeDirection_ ();
  }

  void EdgeDirectionHistogram::setQuantizationLevels (int levels) {
    quantization_levels_ = levels;
  }

  int EdgeDirectionHistogram::getQuantizationLevels () {
    return quantization_levels_;
  }

  cv::Mat EdgeDirectionHistogram::getImage () {
    return getImage (512, 512);
  }

  cv::Mat EdgeDirectionHistogram::getImage (int width, int height) {
    // Normalize the histogram.
    cv::Mat scaled_histogram;
    cv::normalize (direction_histogram_, scaled_histogram, 0, 255,
        cv::NORM_MINMAX, CV_32FC1, cv::Mat ());

    // Draw the histogram.
    cv::Mat image (height, width, CV_8UC3, cv::Scalar (255, 255, 255));
    int rect_width = width / getQuantizationLevels ();

    for (int i = 0; i < scaled_histogram.rows; i++) {
      cv::rectangle (
          image,
          cv::Point (i * rect_width, height),
          cv::Point (
            (int) ((i + 1) * rect_width),
            (int) ((255 - scaled_histogram.at<float> (i, 0)) / 255.0 * height)
          ),
          cv::Scalar (0, 0, 0),
          1, CV_AA, 0
      );
    }

    return image;
  }

  int EdgeDirectionHistogram::getBinValue (int bin) {
    if (bin > getBinCount ()) {
      throw std::runtime_error ("Requested bin exceeds bin count");
    }

    return direction_histogram_.at<int> (bin, 0);
  }

  int EdgeDirectionHistogram::getBinCount () {
    return getQuantizationLevels ();
  }

  cv::Mat EdgeDirectionHistogram::getHistogram () {
    return direction_histogram_.clone ();
  }

  void EdgeDirectionHistogram::convertToLuminance_ () {
    cv::cvtColor (image_, computed_image_, CV_BGR2YCrCb, 3);
    
    std::vector<cv::Mat> ycrcb_components;
    cv::split (computed_image_, ycrcb_components);

    computed_image_ = ycrcb_components[0];
  }

  void EdgeDirectionHistogram::applySobel_ () {
    cv::Mat gradient_x, gradient_y, gradient;

    // Don't use CV_8U because it will result in truncated derivatives.
    cv::Sobel (computed_image_, gradient_x, CV_16S, 1, 0);
    cv::convertScaleAbs (gradient_x, gradient_x_);

    cv::Sobel (computed_image_, gradient_y, CV_16S, 0, 1);
    cv::convertScaleAbs (gradient_y, gradient_y_);

    // Combine the two derivatives in a final image.
    cv::addWeighted (gradient_x_, 0.5, gradient_y_, 0.5, 0, gradient_);
  }

  void EdgeDirectionHistogram::computeDirection_ () {
    // Compute the direction of the gradient vectors.
    direction_ = cv::Mat (gradient_.rows, gradient_.cols, CV_64FC1);
    for (int i = 0; i < gradient_.rows; i++) {
      for (int j = 0; j < gradient_.cols; j++) {
        direction_.at<double> (i, j) = std::atan2 (
            gradient_y_.at<schar> (i, j),
            gradient_x_.at<schar> (i, j)
        );
      }
    }
  }

  void EdgeDirectionHistogram::quantizeDirection_ () {
    direction_histogram_ = cv::Mat (
        getQuantizationLevels (),
        1,
        CV_32SC1,
        cv::Scalar (0)
    );

    double pi = std::atan (1) * 4;

    for (int i = 0; i < direction_.rows; i++) {
      for (int j = 0; j < direction_.cols; j++) {
        for (int k = 0; k < getQuantizationLevels (); k++) {
          double lower = -pi + k * (2 * pi / getQuantizationLevels ());
          double upper = -pi + (k + 1) * (2 * pi / getQuantizationLevels ());
          double direction = direction_.at<double> (i, j);

          if (direction > lower && direction <= upper) {
            int count = direction_histogram_.at<int> (k, 0);
            direction_histogram_.at<int> (k, 0) = count + 1;
          }
        }
      }
    }
  }
}

/* vim: set ts=2 sw=2 et: */
