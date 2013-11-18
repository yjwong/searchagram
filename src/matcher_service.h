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

#ifndef SEARCHAGRAM_MATCHER_SERVICE_H_
#define SEARCHAGRAM_MATCHER_SERVICE_H_

#include "boost/asio.hpp"
#include "matcher_service_connection.h"

namespace SearchAGram {
  class MatcherService {
  public:
    MatcherService (boost::asio::io_service& io_service);

  private:
    const std::string CONFIG_PORT_KEY = "searchagram.matcher.service.port";

    short port_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::io_service& io_service_;

    void start_accept_ ();
    void handle_accept_ (MatcherServiceConnection* new_connection, 
        const boost::system::error_code& error);
  };
}

#endif /* SEARCHAGRAM_MATCHER_SERVICE_H_ */

/* vim: set ts=2 sw=2 et: */
