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

#ifndef SEARCHAGRAM_INDEX_MANAGER_H_
#define SEARCHAGRAM_INDEX_MANAGER_H_

#include <string>
#include <fstream>

#include "boost/thread/mutex.hpp"
#include "opencv2/opencv.hpp"

#include "instagram_image.h"
#include "soci.h"

namespace SearchAGram {

  class IndexManager {
  public:
    static IndexManager& getInstance ();

    soci::session& obtainSession ();
    void releaseSession ();
    
    cv::flann::Index loadFlannIndex ();
    void storeFlannIndex (const cv::flann::Index& index);

    void createInstagramUser (const InstagramUser& user);
    void createInstagramImage (const InstagramImage& image);
    void createVector (const std::string& name, const InstagramImage& image,
        const cv::Mat& vector);
    
    std::vector<cv::Mat> getVectorsWithName (const std::string& name);

  private:
    const std::string CONFIG_PURGE_KEY = "searchagram.index_manager.purge";
    const std::string CONFIG_BACKEND_KEY = "searchagram.index_manager.backend";
    const std::string CONFIG_BACKEND_SQLITE3_FILE_KEY =
      "searchagram.index_manager.backends.sqlite3.file";
    const std::string CONFIG_BACKEND_SQLITE3_SYNCHRONOUS_KEY =
      "searchagram.index_manager.backends.sqlite3.synchronous";
    const std::string CONFIG_BACKEND_SQLITE3_JOURNAL_MODE_KEY =
      "searchagram.index_manager.backends.sqlite3.journal_mode";
    const std::string CONFIG_VECTOR_STORAGE_DIR_KEY =
      "searchagram.index_manager.vector_storage.dir";
    const std::string CONFIG_FLANN_INDEX_FILE_KEY = 
      "searchagram.index_manager.flann_index.file";

    std::string backend_;
    soci::session session_;
    boost::mutex lock_;
    std::fstream vector_storage_;

    IndexManager ();
    ~IndexManager ();

    IndexManager (const IndexManager& other) = delete;
    IndexManager& operator= (const IndexManager&) = delete;

    void initBackend_ ();
    void createTables_();
  };

}

#endif /* SEARCHAGRAM_INDEX_MANAGER_H_ */

/* vim: set ts=2 sw=2 et: */
