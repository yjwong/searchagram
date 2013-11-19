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

#include "boost/log/trivial.hpp"
#include "boost/filesystem.hpp"
#include "network/uri.hpp"

#include "config.h"
#include "next_data_source.h"

namespace filesystem = boost::filesystem;

namespace SearchAGram {

  NextDataSource::NextDataSource () {
    metadata_dir_ = Config::get<std::string> (CONFIG_METADATA_DIR_KEY);
    images_dir_ = Config::get<std::string> (CONFIG_IMAGES_DIR_KEY);

    verifyPaths_ ();
    findMetadataFiles_ ();

    // Initialize the first file.
    curr_metadata_file_ = metadata_files_.begin ();
    if (curr_metadata_file_ != metadata_files_.end ()) {
      readMetadataFile_ (*curr_metadata_file_);
      curr_instagram_image_ = instagram_images_.begin ();
    }
  }

  InstagramImage NextDataSource::getNext () {
    if (!hasMore ()) {
      throw std::runtime_error ("attempt to read past the end of data source");
    }

    InstagramImage retval = *curr_instagram_image_;
    curr_instagram_image_++;

    if (curr_instagram_image_ == instagram_images_.end ()) {
      curr_metadata_file_++;
      if (curr_metadata_file_ != metadata_files_.end ()) {
        readMetadataFile_ (*curr_metadata_file_);
        curr_instagram_image_ = instagram_images_.begin ();
      }
    }

    return retval;
  }

  bool NextDataSource::hasMore () {
    return (curr_metadata_file_ != metadata_files_.end () ||
        curr_instagram_image_ != instagram_images_.end ());
  }

  cv::Mat NextDataSource::getMat (const InstagramImage& image) {
    std::string image_path = getMatLocation (image);

    // Open the image.
    cv::Mat image_mat = cv::imread (image_path);
    return image_mat;
  }

  std::string NextDataSource::getMatLocation (const InstagramImage& image) {
     // TODO: Support other resolutions if they are available.
    std::string url = image.images.at ("standard_resolution").url;
    network::uri image_uri (url);
    std::string filename (image_uri.path ()->substr (1, std::string::npos));
    std::string subfolder (filename.substr (0, 3));

    // Construct the full path.
    filesystem::path image_path (images_dir_);
    image_path /= subfolder;
    image_path /= filename;

    return image_path.string ();
  }

  void NextDataSource::verifyPaths_ () {
    // Verify the metadata path first.
    filesystem::path metadata_path (metadata_dir_);
    if (!filesystem::exists (metadata_path)) {
      throw std::runtime_error ("metadata path does not exist");
    }

    filesystem::file_status metadata_status = filesystem::status (
        metadata_path);
    if (!filesystem::is_directory (metadata_status)) {
      throw std::runtime_error ("metadata path is not a directory");
    }
   
    // Verify the image path.
    filesystem::path images_path (images_dir_);
    if (!filesystem::exists (images_path)) {
      throw std::runtime_error ("images path does not exist");
    }

    filesystem::file_status images_status = filesystem::status (images_path);
    if (!filesystem::is_directory (images_status)) {
      throw std::runtime_error ("images path is not a directory");
    }

    // Both paths should be valid by here.
    BOOST_LOG_TRIVIAL (info) << "metadata path found: " << metadata_dir_;
    BOOST_LOG_TRIVIAL (info) << "images path found: " << images_dir_;
  }

  void NextDataSource::findMetadataFiles_ () {
    filesystem::directory_iterator end_it;
    for (filesystem::directory_iterator it (metadata_dir_); it != end_it;
        ++it) {
      if (!filesystem::is_regular_file (it->status ())) {
        continue;
      }

      // Check file extension, should end with .txt.
      if (filesystem::extension (it->path ()) == ".txt") {
        metadata_files_.emplace_back (it->path ());
        BOOST_LOG_TRIVIAL (info) << "found metadata file: " <<
          it->path ().string ();
      }
    }
  }

  void NextDataSource::readMetadataFile_ (filesystem::path metadata_path) {
    std::ifstream metadata_file (metadata_path.string ());
    std::string line;
    Json::Reader reader;

    instagram_images_.clear ();
    int line_num = 1;
    int item_count = 0;
    while (std::getline (metadata_file, line)) {
      // Each line contains a JSON string.
      Json::Value root;
      bool success = reader.parse (line, root);
      if (!success) {
        BOOST_LOG_TRIVIAL (warning) << "parse error at line " << line_num;
        continue;
      }

      // Create a new Instagram image to store the values.
      InstagramImage image;

      // Read all kinds of metadata.
      // TODO: Comments, likes and tags are not fully implemented.
      if (!readMetadataId_ (image, root, line_num)) continue;
      if (!readMetadataType_ (image, root, line_num)) continue;
      readMetadataFilter_ (image, root, line_num);
      if (!readMetadataTags_ (image, root, line_num)) continue;
      readMetadataCommentsCount_ (image, root, line_num);
      readMetadataCaption_ (image, root, line_num);
      readMetadataLikesCount_ (image, root, line_num);
      if (!readMetadataLink_ (image, root, line_num)) continue;
      if (!readMetadataUser_ (image, root, line_num)) continue;
      if (!readMetadataCreatedTime_ (image, root, line_num)) continue;
      if (!readMetadataImages_ (image, root, line_num)) continue;
      readMetadataUserHasLiked_ (image, root, line_num);

      // Add to our list of images.
      item_count++;
      instagram_images_.push_back (image);

      // Move to the next line!
      line_num++;
    }

    BOOST_LOG_TRIVIAL (info) << item_count << " metadata items processed";
  }

  bool NextDataSource::readMetadataId_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("id")) {
      BOOST_LOG_TRIVIAL (warning) << "no id (line " << line_num << ")";
      return false;
    }

    if (!root["id"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "id is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.id = root["id"].asString ();
    return true;
  }

  bool NextDataSource::readMetadataType_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("type")) {
      BOOST_LOG_TRIVIAL (warning) << "no type (line " << line_num << ")";
      return false;
    }

    if (!root["type"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "type is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.type = root["type"].asString ();
    if (image.type != "image" && image.type != "video") {
      BOOST_LOG_TRIVIAL (warning) << "type \"" << image.type << "\" is not "
        "recognied (line " << line_num << ")";
      return false;
    }

    return true;
  }

  void NextDataSource::readMetadataFilter_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (root.isMember ("filter")) {
      if (!root["filter"].isString ()) {
        BOOST_LOG_TRIVIAL (warning) << "filter is not a string (line " <<
          line_num << ")";
        image.filter = "";
      } else {
        image.filter = root["filter"].asString ();
      }
    } else {
      image.filter = "";
    }
  }

  bool NextDataSource::readMetadataTags_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (root.isMember ("tags")) {
      if (!root["tags"].isArray ()) {
        BOOST_LOG_TRIVIAL (warning) << "tags is not an array (line " <<
          line_num << ")";
      } else {
        int num_tags = root["tags"].size ();
        for (int i = 0; i < num_tags; i++) {
          Json::Value tag = root["tags"][i];
          if (!tag.isString ()) {
            BOOST_LOG_TRIVIAL (warning) << "tag " << i << " in tags is not "
              "a string (line " << line_num << ")";
            return false;
          }

          image.tags.push_back (tag.asString ());
        }
      }
    }

    return true;
  }

  void NextDataSource::readMetadataCommentsCount_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("comtct")) {
      BOOST_LOG_TRIVIAL (warning) << "no comtct, but continuing anyway "
        "(line " << line_num << ")";
      image.comments_count = 0;
    } else {
      if (!root["comtct"].isNumeric ()) {
        BOOST_LOG_TRIVIAL (warning) << "comtct is not a number, but "
         "continuing anyway (line " << line_num << ")";
        image.comments_count = 0;
      } else {
        image.comments_count = root["comtct"].asInt ();
      }
    }
  }

  void NextDataSource::readMetadataCaption_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("caption")) {
      BOOST_LOG_TRIVIAL (warning) << "no caption, but continuing anyway "
        "(line " << line_num << ")";
      image.caption = "";
    } else {
      if (!root["caption"].isObject ()) {
        BOOST_LOG_TRIVIAL (warning) << "caption is not an object, but "
          "continuing anyway (line " << line_num << ")";
        image.caption = "";
      } else {
        Json::Value caption = root["caption"];
        if (!caption.isMember ("text")) {
          BOOST_LOG_TRIVIAL (warning) << "no caption.text, but continuing "
            "anyway (line " << line_num << ")";
          image.caption = "";
        } else {
          if (!caption["text"].isString ()) {
            BOOST_LOG_TRIVIAL (warning) << "caption.text is not a string, "
              "but continuing anyway (line " << line_num << ")";
            image.caption = "";
          } else {
            image.caption = caption["text"].asString ();
          }
        }
      }
    }
  }

  void NextDataSource::readMetadataLikesCount_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("likes")) {
      BOOST_LOG_TRIVIAL (warning) << "no likes, but continuing anyway "
        "(line " << line_num << ")";
      image.likes_count = 0;
    } else {
      if (!root["likes"].isObject ()) {
        BOOST_LOG_TRIVIAL (warning) << "likes is not an object, but "
          "continuing anyway (line " << line_num << ")";
        image.likes_count = 0;
      } else {
        Json::Value likes = root["likes"];
        if (!likes.isMember ("count")) {
          BOOST_LOG_TRIVIAL (warning) << "no likes.count, but continuing "
            "anyway (line " << line_num << ")";
          image.likes_count = 0;
        } else {
          if (!likes["count"].isNumeric ()) {
            BOOST_LOG_TRIVIAL (warning) << "likes.count is not a number, but "
              "continuing anyway (line " << line_num << ")";
            image.likes_count = 0;
          } else {
            image.likes_count = likes["count"].asInt ();
          }
        }
      }
    }
  }

  bool NextDataSource::readMetadataLink_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("refur")) {
      BOOST_LOG_TRIVIAL (warning) << "no refur (line " << line_num << ")";
      return false;
    }

    if (!root["refur"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "refur is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.link = root.get ("refur", "").asString ();
    return true;
  }

  bool NextDataSource::readMetadataUser_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("user")) {
      BOOST_LOG_TRIVIAL (warning) << "no user (line " << line_num << ")";
      return false;
    }

    if (!root["user"].isObject ()) {
      BOOST_LOG_TRIVIAL (warning) << "user is not an object (line " <<
        line_num << ")";
      return false;
    }

    Json::Value user = root["user"];

    // Field: user.id.
    if (!user.isMember ("id")) {
      BOOST_LOG_TRIVIAL (warning) << "no user.id (line " << line_num << ")";
      return false;
    }

    if (!user["id"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "user.id is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.user.id = user["id"].asString ();

    // Field: user.username.
    if (!user.isMember ("name")) {
      BOOST_LOG_TRIVIAL (warning) << "no user.username (line " << line_num <<
        ")";
      return false;
    }

    if (!user["name"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "user.name is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.user.username = user["name"].asString ();

    // Field: user.full_name.
    if (!user.isMember ("fname")) {
      BOOST_LOG_TRIVIAL (warning) << "no user.fname (line " << line_num <<
        ")";
      return false;
    }

    if (!user["fname"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "user.fname is not a string (line " <<
        line_num << ")";
      return false;
    }

    image.user.full_name = user["fname"].asString ();

    // Field: user.profile_picture.
    if (!user.isMember ("upic")) {
      BOOST_LOG_TRIVIAL (warning) << "no user.upic (line " << line_num <<
        ")";
      return false;
    }

    if (!user["upic"].isString ()) {
      BOOST_LOG_TRIVIAL (warning) << "user.upic is not a string (line " <<
        line_num << ")";
      return false;
    }
    
    image.user.profile_picture = user["upic"].asString ();

    // Field: user.bio.
    if (user.isMember ("bio")) {
      if (!user["bio"].isString ()) {
        BOOST_LOG_TRIVIAL (warning) << "user.bio is not a string, but "
         "continuing anyway (line " << line_num << ")";
        image.user.bio = "";
      } else {
        image.user.bio = user["bio"].asString ();
      }
    } else {
      image.user.bio = "";
    }

    // Field: user.website.
    if (user.isMember ("wbsite")) {
      if (!user["wbsite"].isString ()) {
        BOOST_LOG_TRIVIAL (warning) << "user.wbsite is not a string, but "
         "continuing anyway (line " << line_num << ")";
        image.user.website = "";
      } else {
        image.user.website = user["wbsite"].asString ();
      }
    } else {
      image.user.website = "";
    }

    return true;
  }

  bool NextDataSource::readMetadataCreatedTime_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("crtdt")) {
      BOOST_LOG_TRIVIAL (warning) << "no crtdt (line " << line_num << ")";
      return false;
    }

    if (!root["crtdt"].isNumeric ()) {
      BOOST_LOG_TRIVIAL (warning) << "crtdt is not a number (line " <<
        line_num << ")";
      return false;
    }

    // Instagram actually returns the time as a 32-bit integer, but NeXT
    // somehow appends 3 more zeros to achieve millisecond precision.
    double created_time_double = root["crtdt"].asDouble ();
    image.created_time = static_cast<int> (created_time_double / 1000);

    return true;
  }

  bool NextDataSource::readMetadataImages_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (!root.isMember ("images")) {
      BOOST_LOG_TRIVIAL (warning) << "no images (line " << line_num << ")";
      return false;
    }

    if (!root["images"].isObject ()) {
      BOOST_LOG_TRIVIAL (warning) << "images is not an object (line " <<
        line_num << ")";
      return false;
    }

    Json::Value::Members images = root["images"].getMemberNames ();
    for (Json::Value::Members::const_iterator it = images.cbegin ();
        it != images.cend (); ++it) {
      std::string image_name = *it;
      Json::Value json_source_image = root["images"][image_name];
      if (!json_source_image.isObject ()) {
        BOOST_LOG_TRIVIAL (warning) << "images." << image_name << " is not "
          "an object, but continuing anyway (line " << line_num << ")";
        continue;
      }

      // Populate the attributes of the source image.
      InstagramImage::SourceImage source_image;

      // Field: source_image.url.
      if (!json_source_image.isMember ("url")) {
        BOOST_LOG_TRIVIAL (warning) << "no images." << image_name << ".url, "
          "but continuing anyway (line " << line_num << ")";
        continue;
      }

      if (!json_source_image["url"].isString ()) {
        BOOST_LOG_TRIVIAL (warning) << "images." << image_name << ".url is "
          "not a string, but continuing anyway (line " << line_num << ")";
        continue;
      }

      source_image.url = json_source_image["url"].asString ();

      // Field: source_image.width.
      if (!json_source_image.isMember ("width")) {
        BOOST_LOG_TRIVIAL (warning) << "no images." << image_name <<
          ".width, but continuing anyway (line " << line_num << ")";
        continue;
      }

      if (!json_source_image["width"].isInt ()) {
        BOOST_LOG_TRIVIAL (warning) << "images." << image_name << ".width "
          "is not an integer, but continuing anyway (line " << line_num <<
          ")";
        continue;
      }

      source_image.width = json_source_image["width"].asInt ();

      // Field: source_image.height.
      if (!json_source_image.isMember ("height")) {
        BOOST_LOG_TRIVIAL (warning) << "no images." << image_name <<
          ".height, but continuing anyway (line " << line_num << ")";
        continue;
      }

      if (!json_source_image["height"].isInt ()) {
        BOOST_LOG_TRIVIAL (warning) << "images." << image_name << ".height "
          "is not an integer, but continuing anyway (line " << line_num <<
          ")";
        continue;
      }

      source_image.height = json_source_image["height"].asInt ();

      // Add to the list of images.
      image.images.insert (std::make_pair (image_name, source_image));
    }

    if (image.images.size () == 0) {
      BOOST_LOG_TRIVIAL (warning) << "images does not contain any images "
        "(line " << line_num << ")";
      return false;
    }

    return true;
  }

  void NextDataSource::readMetadataUserHasLiked_ (InstagramImage& image,
      Json::Value& root, int line_num) {
    if (root.isMember ("uhasliked")) {
      if (!root["uhasliked"].isBool ()) {
        BOOST_LOG_TRIVIAL (warning) << "uhasliked is not a bool (line " <<
          line_num << ")";
        image.user_has_liked = false;
      } else {
        image.user_has_liked = root["uhasliked"].asBool ();
      }
    } else {
      image.user_has_liked = false;
    }
  }

}

/* vim: set ts=2 sw=2 et: */
