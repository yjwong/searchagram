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
#include <unordered_set>

#include "boost/bind.hpp"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/log/trivial.hpp"

#include "opencv2/opencv.hpp"

#include "b64/decode.h"
#include "b64/encode.h"

#include "analysis/surf_vector.h"
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
           
          } else if (action == "autocomplete_hashtags") {
            handle_autocomplete_hashtags_ (root, response);

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
    std::string filter = "";
    std::string date_interval = "lt";
    int date = std::time (NULL);
    std::string likes_interval = "gt";
    int likes = 0;
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

    if (root.isMember ("date")) {
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

    if (root.isMember ("likes")) {
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

    if (root.isMember ("comments")) {
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
      "`source_images`.`url`, `images`.`caption`, `images`.`likes_count`, "
      "`images`.`comments_count` FROM `images`, `source_images` ";
    if (hashtags.size () > 0) {
      query = query + ", `tags` ";
    }

    query = query + "WHERE ";
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
      query = query + "`tags`.`tag` IN (";
      for (std::vector<std::string>::size_type i = 0; i < hashtags.size ();
          i++) {
        query = query + ":hashtag" + boost::lexical_cast<std::string> (i) +
          ", ";
      }

      query = std::string (query.begin (), query.end () - 2);
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
    if (hashtags.size () > 0) {
      query = query + "`images`.`id` = `tags`.`image_id` AND ";
    }
    query = query + "`images`.`id` = `source_images`.`image_id` AND ";
    query = query + "`source_images`.`name` = 'standard_resolution' ";

    // AND clause for hashtags.
    if (hashtags.size () > 0) {
      query = query + "GROUP BY `images`.`created_time` HAVING COUNT "
        "(DISTINCT `tags`.`tag`) = " +
        boost::lexical_cast<std::string> (hashtags.size ()) + " ";
    }
    
    query = query + "LIMIT 0, 30";

    BOOST_LOG_TRIVIAL (info) << "query: " << query;
    
    // Run the query.
    IndexManager& manager = IndexManager::getInstance ();
    soci::session& session = manager.obtainSession ();
   
    // Do parameter substitution.
    // TODO: This part is damn ugly.
    Json::Value results;
    std::string result_id;
    std::string result_link;
    std::string result_url;
    std::string result_caption;
    int result_likes_count;
    int result_comments_count;

    auto temp = (session.prepare << query);
    temp , soci::use (type, "type"), soci::use (user_id, "user_id"),
         soci::use (filter, "filter");
    for (std::vector<std::string>::size_type i = 0; i < hashtags.size (); i++) {
      temp , soci::use (hashtags[i], "hashtag" + boost::lexical_cast<std::string> (i));
    }
    temp , soci::use (date, "date"), soci::use (likes, "likes"),
         soci::use (comments, "comments"),
        soci::into (result_id), soci::into (result_link),
        soci::into (result_url), soci::into (result_caption),
        soci::into (result_likes_count), soci::into (result_comments_count);

    soci::statement stmt (temp);
    stmt.execute ();

    // Collate the results!
    while (stmt.fetch ()) {
      Json::Value result;
      result["id"] = result_id;
      result["link"] = result_link;
      result["url"] = result_url;
      result["caption"] = result_caption;
      result["likes_count"] = result_likes_count;
      result["comments_count"] = result_comments_count;

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
      try {
        cv::Mat image = cv::imdecode (cv::Mat (query), CV_LOAD_IMAGE_COLOR);
        if (image.data == NULL) {
          response["status"] = 33;
          response["description"] = "query is not an image";

        } else {
          // Load our FLANN index.
          IndexManager& manager = IndexManager::getInstance ();
          std::shared_ptr<cv::FlannBasedMatcher> matcher = manager.obtainFlannMatcher ();

          // Perform SURF analysis on query image.
          BOOST_LOG_TRIVIAL (info) << "received query image, performing analysis";
          SurfVector surf_vector (image);
          surf_vector.setMinHessian (1200);
          cv::Mat feature_vectors = surf_vector.detect ();

          // Perform matching.
          BOOST_LOG_TRIVIAL (info) << "performing matching...";
          std::vector<std::vector<cv::DMatch>> matches;
          matcher->knnMatch (feature_vectors, matches, 2);
          BOOST_LOG_TRIVIAL (info) << "matching complete.";

          // Obtain the matched image.
          float nndr_ratio = 0;
          Json::Value results;
          std::unordered_set<int> duplicate_checker;
          soci::session& vectors_session = manager.obtainVectorsSession ();
          soci::session& session = manager.obtainSession ();

          for (std::vector<std::vector<cv::DMatch>>::size_type i = 0;
              i < matches.size (); i++) {
            std::vector<cv::DMatch> match = matches[i];

            // Check for duplicates.
            if (duplicate_checker.find (match[0].imgIdx) !=
                duplicate_checker.end ()) {
              continue;
            }

            // Use Nearest Neighbour Distance Ratio.
            if (match[0].distance <= nndr_ratio * match[1].distance) {
              std::string image_id;
              vectors_session << "SELECT `image_id` FROM `vectors` WHERE `rowid`"
                " = :rowid", soci::use (match[0].imgIdx + 1),
                soci::into (image_id);
             
              std::string link;
              session << "SELECT `link` FROM `images` WHERE `id` = :id", 
                soci::use (image_id), soci::into (link);

              Json::Value result;
              result["id"] = image_id;
              result["link"] = link;
              result["match_imgidx"] = match[0].imgIdx;
              result["match_distance"] = match[0].distance;

              results.append (result);
              duplicate_checker.insert (match[0].imgIdx);
            }
          }

          manager.releaseSession ();
          manager.releaseVectorsSession ();

          response["status"] = 0;
          response["results"] = results;
        }

      } catch (cv::Exception& e) {
        std::string except_message = "OpenCV exception: ";
        except_message.append (e.what ());

        response["status"] = 34;
        response["results"] = except_message;
        BOOST_LOG_TRIVIAL (error) << except_message;
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

  void MatcherServiceConnection::handle_autocomplete_hashtags_ (
      const Json::Value& root, Json::Value& response) {
    if (!root.isMember ("query")) {
      response["status"] = 51;
      response["description"] = "query is missing";

    } else if (!root["query"].isString ()) {
      response["status"] = 52;
      response["description"] = "query is not a string";

    } else {
      // Check how many hashtags we are supposed to autocomplete.
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

        std::string tag;

        soci::statement stmt = (session.prepare <<
            "SELECT DISTINCT `tag` FROM `tags` WHERE `tag` LIKE :tag LIMIT 0," +
            boost::lexical_cast<std::string> (max_entries), soci::use (query),
            soci::into (tag));
        stmt.execute ();

        // Collate the results!
        while (stmt.fetch ()) {
          Json::Value result;
          result["tag"] = tag;
          results.append (result);
        }

        manager.releaseSession ();
      }

      response["status"] = 0;
      response["results"] = results;
    }
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
