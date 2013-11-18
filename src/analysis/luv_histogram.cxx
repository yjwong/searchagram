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

#include <iostream>
#include <stdexcept>
#include <map>
#include "luv_histogram.h"

namespace SearchAGram {
  LuvHistogram::LuvHistogram (const std::string& filename) {
    cv::Mat image = cv::imread (filename, 1);
    if (!image.data) {
      throw std::runtime_error ("Unable to open file to construct histogram");
    }

    _computeHistogram (image);
  } 

  LuvHistogram::LuvHistogram (cv::Mat mat) {
    _computeHistogram (mat);
  }

  cv::Mat LuvHistogram::getImage () {
    return getImage (512, 512);
  }

  cv::Mat LuvHistogram::getImage (int width, int height) {
    std::map<Channel, cv::Mat> histograms;
    histograms.insert (std::make_pair (CHANNEL_L, l_histogram_));
    histograms.insert (std::make_pair (CHANNEL_U, u_histogram_));
    histograms.insert (std::make_pair (CHANNEL_V, v_histogram_));

    cv::Mat image (height, width, CV_8UC3, cv::Scalar (255, 255, 255));
    for (std::map<Channel, cv::Mat>::iterator it = histograms.begin ();
        it != histograms.end (); ++it) {
      cv::Mat histogram = histograms.at (it->first);

      // Determine color of line.
      cv::Scalar color;
      switch (it->first) {
        case CHANNEL_V:
          color = cv::Scalar (255, 0, 0);
          break;
        case CHANNEL_U:
          color = cv::Scalar (0, 255, 0);
          break;
        case CHANNEL_L:
          color = cv::Scalar (0, 0, 255);
          break;
      }

      for (int i = 1; i < histogram.rows; i++ ) {
        cv::line (
            image,
            cv::Point (
                (int) ((i - 1) * width / 256.0),
                (int) ((255 - histogram.at<cv::Vec3f> (i - 1, 0)[0]) / 255.0 * height)
            ),
            cv::Point (
                (int) (i * width / 256.0),
                (int) ((255 - histogram.at<cv::Vec3f> (i, 0)[0]) / 255.0 * height)
            ),
            color,
            1, CV_AA, 0
        );
      }
    }

    return image;
  }

  double LuvHistogram::getBinValue (Channel channel, int bin) {
    if (bin >= getBinCount ()) {
      throw std::runtime_error ("Requested bin exceeds bin count");
    }

    // Find the desired histogram.
    cv::Mat histogram;
    switch (channel) {
      case CHANNEL_L:
        histogram = l_histogram_;
        break;
      case CHANNEL_U:
        histogram = u_histogram_;
        break;
      case CHANNEL_V:
        histogram = v_histogram_;
        break;
    }

    return histogram.at<double> (bin, 0, 0);
  }

  int LuvHistogram::getBinCount () {
    return v_histogram_.rows;
  }

  cv::Mat LuvHistogram::getHistogram (Channel channel) {
    cv::Mat result;
    switch (channel) {
      case CHANNEL_L:
        result = l_histogram_.clone ();
        break;
      case CHANNEL_U:
        result = u_histogram_.clone ();
        break;
      case CHANNEL_V:
        result = v_histogram_.clone ();
        break;
    }

    return result;
  }

  void LuvHistogram::_computeHistogram (cv::Mat mat) {
    l_histogram_ = _computeHistogramForChannel (mat, CHANNEL_L);
    u_histogram_ = _computeHistogramForChannel (mat, CHANNEL_U);
    v_histogram_ = _computeHistogramForChannel (mat, CHANNEL_V);
  }

  cv::Mat LuvHistogram::_computeHistogramForChannel (cv::Mat image,
      Channel channel) {
    cv::Mat luv_image;
    cv::cvtColor (image, luv_image, CV_BGR2Luv);

    // Separate the image into Luv planes.
    std::vector<cv::Mat> bgr_planes;
    cv::split (luv_image, bgr_planes);

    // Set histogram parameters.
    const int channels = 0;
    const int size[] = { 256, 256 };
    const float range[] = { 0.0f, 256.0f };
    const float* ranges[] = { range };
    cv::Mat histogram;
    cv::calcHist (
        &bgr_planes[channel],
        1,
        &channels,
        cv::Mat (),
        histogram,
        1, 
        size,
        ranges
    );

    // Normalize histogram values.
    cv::normalize (histogram, histogram, 0, 255, cv::NORM_MINMAX, -1,
        cv::Mat ());

    return histogram;
  }
}

/* vim: set ts=2 sw=2 et: */
