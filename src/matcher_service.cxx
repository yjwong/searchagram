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

#include "boost/bind.hpp"
#include "boost/log/trivial.hpp"

#include "config.h"
#include "matcher_service.h"

namespace asio = boost::asio;

namespace SearchAGram {
  MatcherService::MatcherService (asio::io_service& io_service) :
      acceptor_ (io_service, asio::ip::tcp::endpoint (
          asio::ip::tcp::v4 (), port_
      )),
      io_service_ (io_service) {
    port_ = Config::get<short> (CONFIG_PORT_KEY);
    start_accept_ ();
  }

  void MatcherService::start_accept_ () {
    MatcherServiceConnection* new_connection = new MatcherServiceConnection (
        io_service_
    );

    acceptor_.async_accept (new_connection->getSocket (),
        boost::bind (&MatcherService::handle_accept_, this, new_connection,
          asio::placeholders::error)
    );

    BOOST_LOG_TRIVIAL (info) << "Started accepting connections from port " <<
      port_;
  }

  void MatcherService::handle_accept_ (MatcherServiceConnection* new_connection, 
      const boost::system::error_code& error) {
    if (!error) {
      new_connection->start ();
    } else {
      delete new_connection;
    }

    start_accept_ ();
  }
}

/* vim: set ts=2 sw=2 et: */
