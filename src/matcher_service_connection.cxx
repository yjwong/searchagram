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
#include "boost/lexical_cast.hpp"
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

          } else if (action == "image_search") {
            handle_image_search_ (root, response);

          } else if (action == "get_filters") {
            handle_get_filters_ (root, response);
           
          } else if (action == "autocomplete_users") {
            handle_autocomplete_users_ (root, response);

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
    // Set some sane defaults.
    std::string type = "all";
    std::string username = "";
    std::vector<std::string> hashtags;
    std::string filter = "Normal";
    std::string date_interval = "lt";
    int date = std::time (NULL);
    std::string likes_interval = "gt";
    int likes = -1;
    std::string comments_interval = "gt";
    int comments = -1;

    // Perform type validation.
    if (root.isMember ("type") && root["type"].isString ()) {
      type = root["type"].asString ();
      if (type != "all" && type != "video" && type != "image") {
        response["status"] = 11;
        response["description"] = "type must be either all, video or image";
        return;
      }
    }

    if (root.isMember ("username")) {
      if (root["username"].isString ()) {
        username = root["username"].asString ();
      } else {
        response["status"] = 12;
        response["description"] = "username must be a string";
        return;
      }
    }

    if (root.isMember ("hashtags")) {
      if (root["hashtags"].isArray ()) {
        for (Json::ArrayIndex i = 0; i < root["hashtags"].size (); i++) {
          Json::Value hashtag = root["hashtags"][i];
          if (!hashtag.isString ()) {
            response["status"] = 14;
            response["description"] = "hashtags must be an array containing "
              "strings";
            return;
          } else {
            hashtags.push_back (hashtag.asString ());
          }
        }

      } else {
        response["status"] = 13;
        response["description"] = "hashtags must be an array";
        return;
      }
    }

    if (root.isMember ("filter")) {
      if (root["filter"].isString ()) {
        filter = root["filter"].asString ();
      } else {
        response["status"] = 15;
        response["description"] = "filter must be a string";
        return;
      }
    }

    if (root.isMember ("date_interval")) {
      if (root["date_interval"].isString ()) {
        date_interval = root["date_interval"].asString ();
        if (date_interval != "gt" && date_interval != "lt" &&
            date_interval != "eq") {
          response["status"] = 16;
          response["description"] = "date_interval must be either gt, lt or eq";
          return;
        }
      } else {
        response["status"] = 17;
        response["description"] = "date_interval must be a string";
        return;
      }
    }

    if (root.isMember ("date") && root["date"].size () > 0) {
      if (root["date"].isInt ()) {
        date = root["date"].asInt ();
      } else {
        response["status"] = 18;
        response["description"] = "date must be an integer";
        return;
      }
    }

    if (root.isMember ("likes_interval")) {
      if (root["likes_interval"].isString ()) {
        likes_interval = root["likes_interval"].asString ();
        if (likes_interval != "gt" && likes_interval != "lt" &&
            likes_interval != "eq") {
          response["status"] = 19;
          response["description"] = "likes_interval must be either gt, lt or eq";
          return;
        }
      } else {
        response["status"] = 20;
        response["description"] = "likes_interval must be a string";
        return;
      }
    }

    if (root.isMember ("likes") && root["likes"].size () > 0) {
      if (root["likes"].isInt ()) {
        likes = root["likes"].asInt ();
      } else {
        response["status"] = 21;
        response["description"] = "likes must be an integer";
        return;
      }
    }

    if (root.isMember ("comments_interval")) {
      if (root["comments_interval"].isString ()) {
        comments_interval = root["comments_interval"].asString ();
        if (comments_interval != "gt" && comments_interval != "lt" &&
            comments_interval != "eq") {
          response["status"] = 22;
          response["description"] = "comments_interval must be either gt, lt or eq";
          return;
        }
      } else {
        response["status"] = 23;
        response["description"] = "comments_interval must be a string";
        return;
      }
    }

    if (root.isMember ("comments") && root["comments"].size () > 0) {
      if (root["comments"].isInt ()) {
        comments = root["comments"].asInt ();
      } else {
        response["status"] = 24;
        response["description"] = "comments must be an integer";
        return;
      }
    }

    // Resolve the username.
    std::string user_id;
    if (username.length () > 0) {
      IndexManager& manager = IndexManager::getInstance ();
      soci::session& session = manager.obtainSession ();
      session << "SELECT `id` FROM `users` WHERE `username` = :username",
        soci::use (username), soci::into (user_id);

      if (!session.got_data ()) {
        // No such user, and therefore no results.
        response["status"] = 0;
        response["results"] = Json::Value ();
        manager.releaseSession ();
        return;
      }
      
      manager.releaseSession ();
    }

    // Build the query.
    std::string query = "SELECT DISTINCT `images`.`id`, `images`.`link`, "
      "`source_images`.`url` FROM `images`, `tags`, `source_images` WHERE ";
    if (type != "all") {
      query = query + "`type` = :type AND ";
    } else {
      query = query + ":type = :type AND ";
    }

    if (user_id.length () > 0) {
      query = query + "`user_id` = :user_id AND ";
    } else {
      query = query + ":user_id = :user_id AND ";
    }

    if (hashtags.size () > 0) {
      query = query + "(";
      for (const std::string& hashtag : hashtags) {
        query = query + "`tags`.`tag` = '" + hashtag + "' OR ";
      }

      query = std::string (query.begin (), query.end () - 4);
      query = query + ") AND ";
    }

    if (filter.length () > 0) {
      query = query + "`filter` = :filter AND ";
    } else {
      query = query + ":filter = :filter AND ";
    }

    query = query + "`created_time` ";
    if (date_interval == "lt") {
      query = query + "< ";
    } else if (date_interval == "gt") {
      query = query + "> ";
    } else {
      query = query + "= ";
    }
    query = query + ":date AND ";

    query = query + "`likes_count` ";
    if (likes_interval == "lt") {
      query = query + "< ";
    } else if (likes_interval == "gt") {
      query = query + "> ";
    } else {
      query = query + "= ";
    }
    query = query + ":likes AND ";

    query = query + "`comments_count` ";
    if (comments_interval == "lt") {
      query = query + "< ";
    } else if (comments_interval == "gt") {
      query = query + "> ";
    } else {
      query = query + "= ";
    }
    query = query + ":comments AND ";
    query = query + "`images`.`id` = `tags`.`image_id` AND ";
    query = query + "`images`.`id` = `source_images`.`image_id` AND ";
    query = query + "`source_images`.`name` = 'thumbnail' LIMIT 0, 30";

    BOOST_LOG_TRIVIAL (info) << "query: " << query;
    
    // Run the query.
    IndexManager& manager = IndexManager::getInstance ();
    soci::session& session = manager.obtainSession ();
    
    Json::Value results;
    std::string result_id;
    std::string result_link;
    std::string result_url;
    soci::statement stmt = (session.prepare << query,
        soci::use (type, "type"), soci::use (user_id, "user_id"), 
        soci::use (filter, "filter"), soci::use (date, "date"),
        soci::use (likes, "likes"), soci::use (comments, "comments"),
        soci::into (result_id), soci::into (result_link),
        soci::into (result_url));
    stmt.execute ();

    // Collate the results!
    while (stmt.fetch ()) {
      Json::Value result;
      result["id"] = result_id;
      result["link"] = result_link;
      result["url"] = result_url;

      results.append (result);
    }

    manager.releaseSession ();

    response["results"] = results;
    response["status"] = 0;
  }

  void MatcherServiceConnection::handle_image_search_ (const Json::Value& root,
      Json::Value& response) {
    if (!root.isMember ("query")) {
      response["status"] = 31;
      response["description"] = "query is missing";
      
    } else if (!root["query"].isString ()) {
      response["status"] = 32;
      response["description"] = "query is not a string";

    } else {
      // Convert query from base 64 to vector.
      std::vector<char> query = base64_to_vec (root["query"].asString ());

      // Perform image analysis.
      cv::Mat image = cv::imdecode (cv::Mat (query), CV_LOAD_IMAGE_COLOR);
      if (image.data == NULL) {
        response["status"] = 33;
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

  void MatcherServiceConnection::handle_get_filters_ (
      const Json::Value& root, Json::Value& response) {
    // Retrieve ALL the filters! :D
    IndexManager& manager = IndexManager::getInstance ();
    soci::session& session = manager.obtainSession ();
    
    Json::Value results;
    std::string filter;

    soci::statement stmt = (session.prepare <<
        "SELECT DISTINCT `filter` FROM `images`", soci::into (filter));
    stmt.execute ();

    // Collate the results!
    while (stmt.fetch ()) {
      Json::Value result = filter;
      results.append (result);
    }

    manager.releaseSession ();
    response["status"] = 0;
    response["results"] = results;
  }

  void MatcherServiceConnection::handle_autocomplete_users_ (
      const Json::Value& root, Json::Value& response) {
    if (!root.isMember ("query")) {
      response["status"] = 41;
      response["description"] = "query is missing";

    } else if (!root["query"].isString ()) {
      response["status"] = 42;
      response["description"] = "query is not a string";

    } else {
      // Check how many users we are supposed to autocomplete.
      int max_entries = 10;
      if (root.isMember ("max_entries") && root["max_entries"].isInt ()) {
        max_entries = root["max_entries"].asInt ();
      }

      // If query length is too short, we don't return results.
      Json::Value results;
      std::string query = root["query"].asString ();
      if (query.length () >= 1) {
        // Fetch users in ascending order, limit to number of results.
        query = "%" + query + "%";
        IndexManager& manager = IndexManager::getInstance ();
        soci::session& session = manager.obtainSession ();

        std::string username;
        std::string full_name;
        std::string profile_picture;

        soci::statement stmt = (session.prepare <<
            "SELECT `username`, `full_name`, `profile_picture` FROM "
            "`users` WHERE `username` LIKE :username LIMIT 0," +
            boost::lexical_cast<std::string> (max_entries), soci::use (query),
            soci::into (username), soci::into (full_name),
            soci::into (profile_picture));
        stmt.execute ();

        // Collate the results!
        while (stmt.fetch ()) {
          Json::Value result;
          result["username"] = username;
          result["full_name"] = full_name;
          result["profile_picture"] = profile_picture;

          results.append (result);
        }

        manager.releaseSession ();
      }

      response["status"] = 0;
      response["results"] = results;
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
