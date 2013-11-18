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

#ifndef SEARCHAGRAM_NEXT_IMAGE_SOURCE_H_
#define SEARCHAGRAM_NEXT_IMAGE_SOURCE_H_

#include <vector>
#include "boost/filesystem.hpp"
#include "json/json.h"
#include "data_source.h"

namespace SearchAGram {
  
  class NextDataSource : public DataSource {
  public:
    NextDataSource ();

    virtual InstagramImage getNext ();
    virtual bool hasMore ();
    virtual cv::Mat getMat (const InstagramImage& image);
    virtual std::string getMatLocation (const InstagramImage& image);

  private:
    const std::string CONFIG_METADATA_DIR_KEY =
      "searchagram.indexer.source.next.metadata_dir";
    const std::string CONFIG_IMAGES_DIR_KEY =
      "searchagram.indexer.source.next.images_dir";

    std::string metadata_dir_;
    std::string images_dir_;

    std::vector<boost::filesystem::path> metadata_files_;
    std::vector<InstagramImage> instagram_images_;

    std::vector<boost::filesystem::path>::iterator curr_metadata_file_;
    std::vector<InstagramImage>::iterator curr_instagram_image_;

    void verifyPaths_ ();
    void findMetadataFiles_ ();
    void readMetadataFile_ (boost::filesystem::path metadata_path);
    
    bool readMetadataId_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataType_ (InstagramImage& image, Json::Value& root,
        int line_num);
    void readMetadataFilter_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataTags_ (InstagramImage& image, Json::Value& root,
        int line_num);
    void readMetadataCommentsCount_ (InstagramImage& image, Json::Value& root,
        int line_num);
    void readMetadataLikesCount_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataLink_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataUser_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataCreatedTime_ (InstagramImage& image, Json::Value& root,
        int line_num);
    bool readMetadataImages_ (InstagramImage& image, Json::Value& root,
        int line_num);
    void readMetadataUserHasLiked_ (InstagramImage& image, Json::Value& root,
        int line_num);
  };

}

#endif /* SEARCHAGRAM_NEXT_IMAGE_SOURCE_H_ */

/* vim: set ts=2 sw=2 et: */
