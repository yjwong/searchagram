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

#include <sstream>
#include <fstream>

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/log/trivial.hpp"

#include "opencv2/opencv.hpp"

#include "b64/decode.h"
#include "b64/encode.h"

#include "index_manager.h"
#include "matcher.h"
#include "matcher_service_connection.h"

namespace asio = boost::asio;
namespace filesystem = boost::filesystem;

namespace SearchAGram {
  MatcherServiceConnection::MatcherServiceConnection (
      asio::io_service& io_service) : socket_ (io_service) {
  }

  asio::ip::tcp::socket& MatcherServiceConnection::getSocket () {
    return socket_;
  }

  void MatcherServiceConnection::start () {
    asio::async_read_until (socket_, buffer_, '\n',
        boost::bind (&MatcherServiceConnection::handle_read_, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred)
    );
  }

  void MatcherServiceConnection::handle_read_ (
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    if (!error) {
      asio::streambuf::const_buffers_type buffer_data = buffer_.data ();
      std::vector<char> data (
          asio::buffers_begin (buffer_data),
          asio::buffers_end (buffer_data)
      );

      // Check for NULL bytes.
      if (std::find (data.begin (), data.end (), '\0') != data.end ()) {
        BOOST_LOG_TRIVIAL (error) << "encounted NULL byte while reading data";
        socket_.shutdown (asio::ip::tcp::socket::shutdown_both);
        socket_.close ();
        return;
      }

      // At this point we have (roughly) sanitized data.
      std::string current_data (data.begin (), data.end ());
      clean_buffer_.append (current_data);
      buffer_.consume (bytes_transferred);

      // Attempt to parse data as JSON. If the parsing failed, then we may not
      // have received the complete data yet.
      Json::Value root;
      Json::Reader reader;

      if (reader.parse (clean_buffer_, root)) {
        Json::Value response;

        if (!root.isObject ()) {
          response["status"] = 1;
          response["description"] = "JSON given is not an object";

        } else if (!root.isMember ("action")) {
          response["status"] = 2;
          response["description"] = "action is missing";

        } else if (!root["action"].isString ()) {
          response["status"] = 3;
          response["description"] = "action is not a string";

        } else {
          std::string action = root["action"].asString ();

          if (action == "search") {
            handle_search_ (root, response);

          } else {
            response["status"] = 4;
            response["description"] = "unknown action";
          }
        }

        asio::async_write (socket_, asio::buffer (response.toStyledString ()),
            boost::bind (&MatcherServiceConnection::handle_write_, this,
              asio::placeholders::error,
              asio::placeholders::bytes_transferred)
        );

      } else {
        asio::async_read_until (socket_, buffer_, '\n',
            boost::bind (&MatcherServiceConnection::handle_read_, this,
              asio::placeholders::error,
              asio::placeholders::bytes_transferred)
        );
      }

    } else {
      delete this;
    }
  }

  void MatcherServiceConnection::handle_search_ (const Json::Value& root,
      Json::Value& response) {
    if (!root.isMember ("query")) {
      response["status"] = 11;
      response["description"] = "query is missing";
      
    } else if (!root["query"].isString ()) {
      response["status"] = 12;
      response["description"] = "query is not a string";

    } else {
      // Convert query from base 64 to vector.
      std::vector<char> query = base64_to_vec (root["query"].asString ());

      // Perform image analysis.
      cv::Mat image = cv::imdecode (cv::Mat (query), CV_LOAD_IMAGE_COLOR);
      if (image.data == NULL) {
        response["status"] = 13;
        response["description"] = "query is not an image";

      } else {
        // Load our FLANN index.
        IndexManager& manager = IndexManager::getInstance ();
        cv::flann::Index index = manager.loadFlannIndex ();

        response["status"] = -1;
        response["description"] = "not implemented yet";

        // TODO: Implement search.
        //response["status"] = 0;
        //response["results"] = results;
      }
    }
  }

  void MatcherServiceConnection::handle_write_ (
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    if (!error) {
      socket_.shutdown (boost::asio::socket_base::shutdown_both);
      socket_.close ();
    }

    delete this;
  }

  std::vector<char> MatcherServiceConnection::base64_to_vec (
      std::string data) {
    base64::decoder decoder;
    std::stringstream data_ss (data);
    std::stringstream decoded_ss;
   
    // Decode the base64 string.
    decoder.decode (data_ss, decoded_ss);
    std::string decoded_string = decoded_ss.str ();
    std::vector<char> decoded (
        decoded_string.begin (),
        decoded_string.end ()
    );

    return decoded;
  }
  
}

/* vim: set ts=2 sw=2 et: */
