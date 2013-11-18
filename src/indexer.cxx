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
#include <unistd.h>

#include "boost/asio.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/filesystem.hpp"
#include "boost/log/trivial.hpp"
#include "boost/thread.hpp"

#include "opencv2/opencv.hpp"

#include "analysis/rgb_histogram.h"
#include "analysis/surf_vector.h"

#include "next_data_source.h"
#include "index_manager.h"
#include "config.h"
#include "indexer.h"

namespace program_options = boost::program_options;
namespace filesystem = boost::filesystem;
namespace asio = boost::asio;

namespace SearchAGram {

  // This is the default configuration file path.
  std::string Indexer::config_ = "../config/searchagram.xml";

  int Indexer::parseArguments_ (int argc, char* argv[]) {
    // Detail the allowable options.
    program_options::options_description desc ("Allowed options");
    desc.add_options ()
      ("help", "Shows this help message.")
      ("config", program_options::value<std::string> (), "Specifies the path "
       "to a configuration file. If not specified, it defaults to "
       "config/indexer.ini (relative to the current working directory). If the "
       "configuration file is not found, then the program exits.");

    program_options::variables_map vm;

    try {
      program_options::store (
          program_options::parse_command_line (argc, argv, desc), vm
      );

    } catch (program_options::error& e) {
      std::cout << e.what () << std::endl;
      std::cout << std::endl;
      std::cout << desc << std::endl;
      return EXIT_PROGRAM_OPTIONS_ERROR;
    }

    // Check if help was called.
    if (vm.count ("help")) {
      std::cout << desc << std::endl;
      return EXIT_HELP_CALLED;
    }

    // Notify if there are any isses with arguments.
    try {
      program_options::notify (vm);
    } catch (program_options::error& e) {
      std::cout << e.what () << std::endl;
      std::cout << std::endl;
      std::cout << desc << std::endl;
      return EXIT_PROGRAM_OPTIONS_ERROR;
    }

    // Check if config is specified.
    if (vm.count ("config")) {
      config_ = vm["config"].as<std::string> ();
    }

    return EXIT_OK;
  }

  int Indexer::checkConfig_ () {
    filesystem::path file_path (config_);
    boost::system::error_code error_code;

    if (!filesystem::exists (file_path, error_code)) {
      BOOST_LOG_TRIVIAL (error) << "configuration file does not exist";
      return EXIT_CONFIG_NOT_EXIST;
    }

    filesystem::file_status file_status = filesystem::status (file_path);
    if (file_status.type () != filesystem::file_type::regular_file) {
      BOOST_LOG_TRIVIAL (error) << "configuration file is not a regular file";
      return EXIT_CONFIG_NOT_REGULAR_FILE;
    }

    // Attempt to open the file at this point.
    std::fstream file (config_, std::ios::in);
    if (file.fail ()) {
      BOOST_LOG_TRIVIAL (error) << "failed to open configuration file (check "
        "permissions?)";
      return EXIT_CONFIG_NOT_READABLE;
    }

    BOOST_LOG_TRIVIAL (info) << "configuration file found: " << config_;
    return EXIT_OK;
  }

  void Indexer::buildIndex_ () {
    BOOST_LOG_TRIVIAL (info) << "indexing stage 1: analyzing images";

    // Create a thread pool to do the work.
    asio::io_service io_service;
    boost::thread_group threads;
    std::auto_ptr<asio::io_service::work> work (
        new asio::io_service::work (io_service));
    for (std::size_t i = 0; i < boost::thread::hardware_concurrency (); ++i) {
      threads.create_thread (boost::bind (&asio::io_service::run, &io_service));
    }

    // Analyze the NExT data source.
    std::shared_ptr<DataSource> data_source (new NextDataSource ());
    InstagramImage image;
    while (*data_source >> image) {
      io_service.post ([image, data_source] {
        BOOST_LOG_TRIVIAL (info) << "processing " << 
          data_source->getMatLocation (image);

        cv::Mat image_mat = data_source->getMat (image);
        if (image_mat.data == NULL) {
          BOOST_LOG_TRIVIAL (warning) << "error processing " << 
            data_source->getMatLocation (image);
          return;
        }

        // Add the Instagram info to the index.
        IndexManager& manager = IndexManager::getInstance ();
        manager.createInstagramUser (image.user);
        manager.createInstagramImage (image);
        for (std::pair<std::string, InstagramImage::SourceImage> source_image :
            image.images) {
          manager.createInstagramSourceImage (
            source_image.first, 
            source_image.second,
            image);
        }

        // Test: RGB as feature vector.
        RGBHistogram histogram (image_mat);
        cv::Mat r_histogram = histogram.getHistogram (RGBHistogram::CHANNEL_RED);
        manager.createVector ("r_histogram", image, r_histogram);

        // Use SURF to extract feature vectors.
        //SurfVector surf (image_mat);
        //cv::Mat feature_vector = surf.detect ();
        //cv::flann::Index flann_index 
      });
    }
  
    // Wait for all work to be done. 
    work.reset ();
    threads.join_all ();
    BOOST_LOG_TRIVIAL (info) << "indexing stage 1 complete";
  }

  void Indexer::buildFlannIndex_ () {
    BOOST_LOG_TRIVIAL (info) << "indexing stage 2: building FLANN index";

    // Obtain the R histogram.
    IndexManager& manager = IndexManager::getInstance ();
    std::vector<cv::Mat> r_histograms = manager.getVectorsWithName ("r_histogram");

    // Convert the R histogram into a multi-dimensional Mat.
    cv::Mat r_histograms_mat (256, r_histograms.size (), CV_8UC1);
    for (std::vector<cv::Mat>::size_type i = 0; i < r_histograms.size (); i++) {
      r_histograms_mat.col (i) = r_histograms[i];
    }

    // Initialize the FLANN index.
    cv::flann::Index flann_index (
        r_histograms_mat,
        cv::flann::LshIndexParams (20, 10, 2),
        cvflann::FLANN_DIST_EUCLIDEAN
    );

    manager.storeFlannIndex (flann_index);
    BOOST_LOG_TRIVIAL (info) << "indexing stage 2 complete";
  }

  int Indexer::main (int argc, char* argv[]) {
    // Set logging format.
    BOOST_LOG_TRIVIAL (info) << "search-a-gram indexer started";

    // Parse arguments from the command line.
    int retval = parseArguments_ (argc, argv);
    if (retval) {
      return retval;
    }

    // Check configuration files for validity.
    retval = checkConfig_ ();
    if (retval) {
      return retval;
    }
    
    // Populate configuration from file.
    try {
      Config::populateFromFile (config_);
    } catch (boost::property_tree::xml_parser_error& e) {
      BOOST_LOG_TRIVIAL (error) << "failed to parse configuration file: " <<
        e.what ();
      return EXIT_CONFIG_PARSE_ERROR;
    }

    // Build the index.
    try {
      buildIndex_ ();
      buildFlannIndex_ ();

    } catch (boost::property_tree::ptree_bad_path& e) {
      BOOST_LOG_TRIVIAL (error) << "unable to set up data source: " <<
        e.what ();
      return EXIT_DATA_SOURCE_ERROR;
    } catch (std::runtime_error& e) {
      BOOST_LOG_TRIVIAL (error) << "unable to set up data source: " <<
        e.what ();
    }
   
    return EXIT_OK;
  }

}

int main (int argc, char* argv[]) {
  return SearchAGram::Indexer::main (argc, argv);
}

/* vim: set ts=2 sw=2 et: */