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

#include "soci-sqlite3.h"
#include "config.h"
#include "analysis/surf_vector.h"
#include "index_manager.h"

namespace filesystem = boost::filesystem;

namespace SearchAGram {

  IndexManager::IndexManager () :
      flann_index_loaded_ (false) {
    backend_ = Config::get<std::string> (CONFIG_BACKEND_KEY);
    initBackend_ ();
    initVectorsBackend_ ();
  }

  IndexManager::~IndexManager () {
    session_.close ();
  }

  IndexManager& IndexManager::getInstance () {
    static IndexManager instance;
    return instance;
  }

  void IndexManager::initBackend_ () {
    // Obtain database filename.
    std::string filename = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_FILE_KEY);

    // Check if we should purge the old database.
    bool purge = Config::get<bool> (CONFIG_PURGE_KEY);
    if (purge) {
      BOOST_LOG_TRIVIAL (info) << "purging old database file...";
      filesystem::remove (filename);
    }

    // Create a new SQLite 3 database instance.
    session_.open (soci::sqlite3, filename);

    // Create tables if they don't exist.
    createTables_ ();

    // We want high performance.
    std::string synchronous = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_SYNCHRONOUS_KEY);
    session_ << "PRAGMA synchronous = " << synchronous;

    std::string journal_mode = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_JOURNAL_MODE_KEY);
    session_ << "PRAGMA journal_mode = " << journal_mode;
  }

  void IndexManager::initVectorsBackend_ () {
    // Obtain database filename.
    std::string filename = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_VECTORS_KEY);

    // Check if we should purge the old database.
    bool purge = Config::get<bool> (CONFIG_PURGE_KEY);
    if (purge) {
      BOOST_LOG_TRIVIAL (info) << "purging old database file...";
      filesystem::remove (filename);
    }
    
    // Create a new SQLite 3 database instance.
    vectors_session_.open (soci::sqlite3, filename);

    // Create tables if they don't exist.
    createVectorsTables_ ();

    // We want high performance.
    std::string synchronous = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_SYNCHRONOUS_KEY);
    vectors_session_ << "PRAGMA synchronous = " << synchronous;

    std::string journal_mode = Config::get<std::string> (
        CONFIG_BACKEND_SQLITE3_JOURNAL_MODE_KEY);
    vectors_session_ << "PRAGMA journal_mode = " << journal_mode;
  }

  void IndexManager::loadFlannIndex () {
    cv::Ptr<cv::flann::IndexParams> index_params = 
      new cv::flann::KDTreeIndexParams (4);
    flann_matcher_ = std::shared_ptr<cv::FlannBasedMatcher> (
       new cv::FlannBasedMatcher (index_params));

    // TODO: This is hardcoded but we can do better.
    // For SURF.
    std::vector<cv::Mat> surf_vectors = getVectorsWithName ("surf");
    flann_matcher_->add (surf_vectors);
    flann_matcher_->train ();

    // For CCV.
    /*
    std::vector<cv::Mat> ccv_vectors = getVectorsWithName ("ccv");
    flann_matcher_->add (ccv_vectors);
    flann_matcher_->train ();
    */

    // For RGB.
    /*
    std::vector<cv::Mat> rgb_vectors = getVectorsWithName ("rgb");
    flann_matcher_->add (rgb_vectors);
    flann_matcher_->train ();
    */

    /*
    cv::Mat query = cv::imread ("/home/yjwong/Pictures/Screenshot from 2013-01-15 15:07:59.png");
    SurfVector surf_vector (query);
    cv::Mat descriptors = surf_vector.detect ();

    std::vector<std::vector<cv::DMatch>> matches;
    flann_matcher_.knnMatch (descriptors, matches, 20);
    for (int i = 0; i < 20; i++) {
      std::cout << matches[0][i].imgIdx << std::endl;
    }
    */

    // Show some stats!
    BOOST_LOG_TRIVIAL (info) << "trained descriptors count: " <<
      flann_matcher_->getTrainDescriptors ().size ();
    flann_index_loaded_ = true;
  }

  std::shared_ptr<cv::FlannBasedMatcher> IndexManager::obtainFlannMatcher () {
    if (!flann_index_loaded_) {
      loadFlannIndex ();
    }

    return flann_matcher_;
  }

  void IndexManager::storeFlannIndex (const cv::flann::Index& index) {
    std::string flann_file = Config::get<std::string> (
        CONFIG_FLANN_INDEX_FILE_KEY);
    index.save (flann_file);
  }

  soci::session& IndexManager::obtainSession () {
    lock_.lock ();
    return session_;
  }

  void IndexManager::releaseSession () {
    lock_.unlock ();
  }

  soci::session& IndexManager::obtainVectorsSession () {
    vectors_lock_.lock ();
    return vectors_session_;
  }

  void IndexManager::releaseVectorsSession () {
    vectors_lock_.unlock ();
  }

  void IndexManager::createInstagramUser (const InstagramUser& user) {
    lock_.lock ();
    session_ << "INSERT OR IGNORE INTO `users` (`id`, `username`, `full_name`, "
      "`profile_picture`, `bio`, `website`) VALUES (:id, :username, "
      ":full_name, :profile_picture, :bio, :website)",
      soci::use (user);
    lock_.unlock ();
  }

  void IndexManager::createInstagramImage (const InstagramImage& image) {
    lock_.lock ();
    session_ << "INSERT OR IGNORE INTO `images` (`id`, `user_id`, `type`, "
      "`filter`, `comments_count`, `caption`, `likes_count`, `link`, "
      "`created_time`, `user_has_liked`) VALUES (:id, :user_id, :type, "
      ":filter, :comments_count, :caption, :likes_count, :link, :created_time, "
      ":user_has_liked)", soci::use (image);

    for (const std::string& tag : image.tags) {
      session_ << "INSERT OR IGNORE INTO `tags` (`image_id`, `tag`) VALUES "
        "(:image_id, :tag)", soci::use (image.id), soci::use (tag);
    }

    for (const std::pair<std::string, InstagramImage::SourceImage>& pair :
        image.images) {
      session_ << "INSERT OR IGNORE INTO `source_images` (`image_id`, "
        "`name`, `width`, `height`, `url`) VALUES (:image_id, :name, "
        ":width, :height, :url)",
        soci::use (image.id, "image_id"),
        soci::use (pair.first, "name"), 
        soci::use (pair.second);
    }

    lock_.unlock ();
  }

  void IndexManager::createVector (const std::string& name,
      const InstagramImage& image, const cv::Mat& vector) {
    vectors_lock_.lock ();

    // Convert the vector to binary.
    std::vector<char> data (
        vector.data,
        vector.data + (vector.elemSize () * vector.rows * vector.cols)
    );

    // Prepare the blob for insertion.
    soci::blob vector_blob (session_);
    vector_blob.write (0, data.data (), data.size ());

    // Create the vector in SQL.
    vectors_session_ << "INSERT OR IGNORE INTO `vectors` (`image_id`, `name`, "
      "`rows`, `cols`, `elem_size`, `elem_type`, `data`) values (:image_id, "
      ":name, :rows, :cols, :elem_size, :elem_type, :data)",
      soci::use (image.id, "image_id"),
      soci::use (name, "name"),
      soci::use (vector.rows, "rows"),
      soci::use (vector.cols, "cols"),
      soci::use (vector.elemSize (), "elem_size"),
      soci::use (vector.type (), "elem_type"),
      soci::use (vector_blob, "data");
    
    vectors_lock_.unlock ();  
  }

  std::vector<cv::Mat> IndexManager::getVectorsWithName (const std::string& name) {
    vectors_lock_.lock ();

    // Retrieve the vectors.
    std::vector<cv::Mat> results;

    std::string image_id;
    int rows;
    int cols;
    int elem_size;
    int elem_type;
    soci::blob vector_blob (vectors_session_);

    soci::statement stmt = (vectors_session_.prepare << 
      "SELECT `image_id`, `rows`, `cols`, `elem_size`, `elem_type`, "
      "`data` FROM `vectors` WHERE `name` = :name",
      soci::use (name, "name"), soci::into (image_id), soci::into (rows),
      soci::into (cols), soci::into (elem_size), soci::into (elem_type),
      soci::into (vector_blob));
    
    stmt.execute ();
    while (stmt.fetch ()) {
      // Unlock first, we no longer need the session.
      cv::Mat vector (rows, cols, elem_type);
      vector_blob.read (0, (char*) vector.ptr (), vector_blob.get_len ());
      results.push_back (vector);
    }

    // We don't need the session anymore.
    vectors_lock_.unlock ();

    return results;
  }

  void IndexManager::createTables_ () {
    soci::transaction transaction (session_);

    // Create the users table.
    session_ << "CREATE TABLE IF NOT EXISTS `users` ("
      "`id` VARCHAR(128) NOT NULL ,"
      "`username` VARCHAR(64) NOT NULL ,"
      "`full_name` TEXT NOT NULL ,"
      "`profile_picture` TEXT NOT NULL ,"
      "`bio` TEXT NULL ,"
      "`website` TEXT NULL ,"
      "PRIMARY KEY (`id`) )";

    session_ << "CREATE INDEX IF NOT EXISTS `username` ON `users` (`username`)";

    // Create the images table.
    session_ << "CREATE TABLE IF NOT EXISTS `images` ("
      "`id` VARCHAR(128) NOT NULL ,"
      "`user_id` VARCHAR(128) NOT NULL ,"
      "`type` VARCHAR(16) NOT NULL ,"
      "`filter` TEXT NULL ,"
      "`comments_count` INT NULL DEFAULT 0 ,"
      "`caption` TEXT NULL ,"
      "`likes_count` INT NULL DEFAULT 0 ,"
      "`link` TEXT NOT NULL ,"
      "`created_time` DATETIME NOT NULL ,"
      "`user_has_liked` TINYINT(1) NULL DEFAULT false ,"
      "PRIMARY KEY (`id`) ,"
      "CONSTRAINT `fk_images_users`"
      "  FOREIGN KEY (`user_id` )"
      "  REFERENCES `users` (`id` )"
      "  ON DELETE NO ACTION "
      "  ON UPDATE NO ACTION)";
    
    session_ << "CREATE INDEX IF NOT EXISTS `user_id` on `images` "
      "(`user_id` ASC)";

    // Create the tags table.
    session_ << "CREATE TABLE IF NOT EXISTS `tags` ("
      "`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,"
      "`image_id` VARCHAR(128) NOT NULL ,"
      "`tag` VARCHAR(128) NOT NULL ,"
      "CONSTRAINT `fk_tags_images`"
      "  FOREIGN KEY (`image_id` )"
      "  REFERENCES `images` (`id` )"
      "  ON DELETE NO ACTION "
      "  ON UPDATE NO ACTION)";

    session_ << "CREATE INDEX IF NOT EXISTS `image_id` on `tags` "
      "(`image_id` ASC)";
    session_ << "CREATE INDEX IF NOT EXISTS `tag` on `tags` (`tag` ASC)";

    // Create the source images table.
    session_ << "CREATE TABLE IF NOT EXISTS `source_images` ("
      "`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,"
      "`image_id` VARCHAR(128) NOT NULL ,"
      "`name` VARCHAR(32) NOT NULL ,"
      "`width` INT NOT NULL ,"
      "`height` INT NOT NULL ,"
      "`url` TEXT NOT NULL ,"
      "CONSTRAINT `fk_source_images_images`"
      "  FOREIGN KEY (`image_id` )"
      "  REFERENCES `images` (`id` )"
      "  ON DELETE NO ACTION "
      "  ON UPDATE NO ACTION)";

    session_ << "CREATE INDEX IF NOT EXISTS `image_id` on `source_images` "
      "(`image_id` ASC)";

    // Commit the changes.
    transaction.commit ();
  }
  
  void IndexManager::createVectorsTables_ () {
    soci::transaction transaction (vectors_session_);

    // Create the feature vectors table.
    vectors_session_ << "CREATE TABLE IF NOT EXISTS `vectors` ("
      "`image_id` VARCHAR(128) NOT NULL ,"
      "`name` VARCHAR(64) NOT NULL ,"
      "`rows` INT NOT NULL ,"
      "`cols` INT NOT NULL ,"
      "`elem_size` INT NOT NULL ,"
      "`elem_type` INT NOT NULL ,"
      "`data` BLOB NOT NULL)";

    vectors_session_ << "CREATE INDEX IF NOT EXISTS `image_id` on `vectors` "
      "(`image_id` ASC)";
    vectors_session_ << "CREATE INDEX IF NOT EXISTS `name` on `vectors` "
      "(`name` ASC)";

    // Commit the changes.
    transaction.commit ();
  }

}

/* vim: set ts=2 sw=2 et: */
