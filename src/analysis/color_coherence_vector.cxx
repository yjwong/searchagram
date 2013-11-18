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

#include <stdexcept>
#include <queue>
#include <vector>
#include <map>

#include "color_coherence_vector.h"

namespace SearchAGram {
  const int ColorCoherenceVector::DEFAULT_DISCRETIZE_BPC = 2;
  const std::vector<cv::Point>::size_type ColorCoherenceVector::DEFAULT_TAU = 4;

  ColorCoherenceVector::ColorCoherenceVector (const std::string& filename) :
      discretize_bpc_ (DEFAULT_DISCRETIZE_BPC),
      tau_ (DEFAULT_TAU),
      computed_ (false) {
    image_ = cv::imread (filename, 1);
    if (image_.data == NULL) {
      throw std::runtime_error ("Unable to open file to construct vector");
    }

    visited_pixels_ = cv::Mat (image_.rows, image_.cols, CV_8UC1,
        cv::Scalar (0));
  }

  ColorCoherenceVector::ColorCoherenceVector (cv::Mat mat) :
      discretize_bpc_ (DEFAULT_DISCRETIZE_BPC),
      tau_ (DEFAULT_TAU),
      computed_ (false),
      visited_pixels_ (cv::Mat (mat.rows, mat.cols, CV_8UC1, cv::Scalar (0))) {
    image_ = mat;
  }

  void ColorCoherenceVector::setFilterEngine (
      cv::Ptr<cv::FilterEngine> filter_engine) {
    filter_engine_ = filter_engine;
  }

  cv::Ptr<cv::FilterEngine> ColorCoherenceVector::getFilterEngine () {
    return filter_engine_;
  }

  void ColorCoherenceVector::setDiscretizeBPC (int bpc) {
    discretize_bpc_ = bpc;
  }

  int ColorCoherenceVector::getDiscretizeBPC () {
    return discretize_bpc_;
  }

  void ColorCoherenceVector::setTau (std::vector<cv::Point>::size_type tau) {
    tau_ = tau;
  }

  std::vector<cv::Point>::size_type ColorCoherenceVector::getTau () {
    return tau_;
  }

  cv::Mat ColorCoherenceVector::getImage () {
    return getImage (512, 512);
  }

  void ColorCoherenceVector::compute () {
    computed_image_ = image_.clone ();
    applyFilterEngine_ ();

    // Discretize colors.
    discretizeColorSpace_ (discretize_bpc_);

    // Locate regions on image.
    for (int i = 0; i < computed_image_.rows; i++) {
      for (int j = 0; j < computed_image_.cols; j++) {
        if (visited_pixels_.at<uchar> (i, j) == 0) {
          regions_.push_back (findRegionMatchingPixel_ (i, j));
        }
      }
    }

    // Set computed flag.
    computed_ = true;
  }

  cv::Mat ColorCoherenceVector::getCCV () {
    computedCheck_ ();

    int image_bins = 1 << (discretize_bpc_ * 3);
    std::vector<std::vector<int>> ccv;
    for (int i = 0; i < image_bins; i++) {
      std::vector<int> ccv_row;
      ccv_row.push_back (0);
      ccv_row.push_back (0);

      ccv.push_back (ccv_row);
    }

    for (std::vector<std::vector<cv::Point>>::iterator it = regions_.begin ();
        it != regions_.end (); ++it) {
      std::vector<cv::Point> region = *it;
      
      // Identify the color.
      cv::Point pixel = region.at (0);
      int pixel_value =
        ((int) computed_image_.at<cv::Vec3b> (pixel.x, pixel.y)[0] >> (8 - discretize_bpc_)) << (discretize_bpc_ * 2) |
        ((int) computed_image_.at<cv::Vec3b> (pixel.x, pixel.y)[1] >> (8 - discretize_bpc_)) << discretize_bpc_ |
        ((int) computed_image_.at<cv::Vec3b> (pixel.x, pixel.y)[2] >> (8 - discretize_bpc_));
      std::vector<cv::Point>::size_type pixel_count = region.size ();

      // Check if we already have the pixel value in the CCV.
      if (pixel_count > tau_) {
        ccv.at (pixel_value).at (0) += (int) pixel_count;
      } else {
        ccv.at (pixel_value).at (1) += (int) pixel_count;
      }
    }

    cv::Mat ccv_norm (image_bins, 2, CV_64FC1);
    for (int i = 0; i < image_bins; i++) {
      for (int j = 0; j < 2; j++) {
        double normalized = 0;
        if (ccv.at (i).at (j) != 0) {
          normalized = std::log (ccv.at (i).at (j));
        }

        ccv_norm.at<double> (i, j) = normalized;
      }
    }

    // Normalize the CCV.
    cv::normalize (ccv_norm, ccv_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat ());
    return ccv_norm;
  }

  cv::Mat ColorCoherenceVector::getImage (int width, int height) {
    computedCheck_ ();

    cv::Mat image = image_.clone ();
    cv::RNG rng (0xC001D00D);
    for (std::vector<std::vector<cv::Point>>::iterator it = regions_.begin ();
        it != regions_.end (); ++it) {
      cv::Vec3b color (
          rng.uniform (0, 256),
          rng.uniform (0, 256),
          rng.uniform (0, 256)
      );

      for (cv::Point point : *it) {
        image.at<cv::Vec3b> (point.x, point.y) = color;
      }
    }

    return image;
  }

  void ColorCoherenceVector::computedCheck_ () {
    if (!computed_) {
      throw std::runtime_error ("The vector has not been computed yet");
    }
  }

  void ColorCoherenceVector::applyFilterEngine_ () {
    filter_engine_->apply (
        computed_image_,  // The source image
        computed_image_   // The destination image
    );
  }

  void ColorCoherenceVector::discretizeColorSpace_ (int bpc) {
    if (bpc > 8) {
      throw std::runtime_error ("More than 8 bits per channel not supported");
    }

    if (bpc < 1) {
      throw std::runtime_error ("Bits per channel must be more than zero");
    }

    // Store the discretized colors.
    // This eliminates the LSBs on each channel.
    uchar mask = ((0xFF << (8 - bpc)) & 0xFF);
    for (int i = 0; i < computed_image_.rows; i++) {
      for (int j = 0; j < computed_image_.cols; j++) {
        computed_image_.at<cv::Vec3b> (i, j) = cv::Vec3b (
            computed_image_.at<cv::Vec3b> (i, j)[0] & mask,
            computed_image_.at<cv::Vec3b> (i, j)[1] & mask,
            computed_image_.at<cv::Vec3b> (i, j)[2] & mask
        );
      }
    }
  }

  std::vector<cv::Point> ColorCoherenceVector::findRegionMatchingPixel_ (
      int row, int col) {
    std::queue<cv::Point> bfs_queue;
    std::vector<cv::Point> region_points;
    bfs_queue.push (cv::Point (row, col));
    visited_pixels_.at<uchar> (row, col) = 255;

    // BFS connected components search.
    cv::Point* neighbours = new cv::Point[8];
    int left = 0, right = 0, top = 0, bottom = 0;

    while (!bfs_queue.empty ()) {
      cv::Point node = bfs_queue.front ();
      bfs_queue.pop ();

      cv::Vec3b pixelValue = computed_image_.at<cv::Vec3b> (node.x, node.y);
      region_points.push_back (node);

      left = node.x - 1;
      right = node.x + 1;
      top = node.y + 1;
      bottom = node.y - 1;

      neighbours[0].x = left; neighbours[0].y = bottom;
      neighbours[1].x = left; neighbours[1].y = node.y;
      neighbours[2].x = left; neighbours[2].y = top;
      neighbours[3].x = node.x; neighbours[3].y = bottom;
      neighbours[4].x = node.x; neighbours[4].y = top;
      neighbours[5].x = right; neighbours[5].y = bottom;
      neighbours[6].x = right; neighbours[6].y = node.y;
      neighbours[7].x = right; neighbours[7].y = top;

      for (int i = 0; i < 8; i++) {
        cv::Point neighbour = neighbours[i];
        if (neighbour.x >= computed_image_.rows ||
            neighbour.y >= computed_image_.cols ||
            neighbour.x < 0 || neighbour.y < 0) {
          continue;
        }

        pixelValue = computed_image_.at<cv::Vec3b> (neighbour.x, neighbour.y);
        if (visited_pixels_.at<uchar> (neighbour.x, neighbour.y) == 0 &&
            computed_image_.at<cv::Vec3b> (row, col) == pixelValue) {
          visited_pixels_.at<uchar> (neighbour.x, neighbour.y) = 255;
          bfs_queue.push (neighbour);
        }
      }
    }

    delete[] neighbours;
    return region_points;
  }
}

/* vim: set ts=2 sw=2 et: */
