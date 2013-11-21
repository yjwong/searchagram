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
#include <fstream>

#include "boost/asio.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/log/trivial.hpp"

#include "config.h"
#include "index_manager.h"
#include "matcher_service.h"
#include "matcher.h"

namespace asio = boost::asio;
namespace program_options = boost::program_options;
namespace filesystem = boost::filesystem;

namespace SearchAGram {

  // This is the default configuration file path.
  std::string Matcher::config_ = "../config/searchagram.xml";

  int Matcher::parseArguments_ (int argc, char* argv[]) {
    // Detail the allowable options.
    program_options::options_description desc ("Allowed options");
    desc.add_options ()
      ("help", "Shows this help message.")
      ("config", program_options::value<std::string> (), "Specifies the path "
       "to a configuration file. If not specified, it defaults to "
       "config/matcher.ini (relative to the current working directory). If the "
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

  int Matcher::checkConfig_ () {
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

  int Matcher::main (int argc, char* argv[]) {
    // Set logging format.
    BOOST_LOG_TRIVIAL (info) << "search-a-gram matcher started";

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

    // Preload the FLANN index.
    BOOST_LOG_TRIVIAL (info) << "preloading FLANN index...";
    IndexManager& manager = IndexManager::getInstance ();
    manager.loadFlannIndex ();

    // Start the matcher service.
    asio::io_service io_service;
    MatcherService matcher_service (io_service);
    io_service.run ();

    return 0;
  }
}

int main (int argc, char* argv[]) {
  return SearchAGram::Matcher::main (argc, argv);
}

/* vim: set ts=2 sw=2 et: */
