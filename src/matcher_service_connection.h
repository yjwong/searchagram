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

#ifndef SEARCHAGRAM_MATCHER_SERVICE_CONNECTION_H_
#define SEARCHAGRAM_MATCHER_SERVICE_CONNECTION_H_

#include <string>
#include <vector>

#include "json/json.h"
#include "boost/asio.hpp"

namespace SearchAGram {
  class MatcherServiceConnection {
  public:
    MatcherServiceConnection (boost::asio::io_service& io_service);

    boost::asio::ip::tcp::socket& getSocket ();
    void start ();
 
  private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    std::string clean_buffer_;

    void handle_search_ (const Json::Value& root, Json::Value& response);
    void handle_get_filters_ (const Json::Value& root, Json::Value& response);
    void handle_autocomplete_users_ (const Json::Value& root,
        Json::Value& response);

    void handle_write_ (const boost::system::error_code& error,
        size_t bytes_transferred);
    void handle_read_ (const boost::system::error_code& error,
        size_t bytes_transferred);
    std::vector<char> base64_to_vec (std::string data);
  };
}

#endif /* SEARCHAGRAM_MATCHER_SERVICE_CONNECTION_H_ */

/* vim: set ts=2 sw=2 et: */
