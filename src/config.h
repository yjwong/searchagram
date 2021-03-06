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

#ifndef SEARCHAGRAM_CONFIG_H_
#define SEARCHAGRAM_CONFIG_H_

#include <string>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"

namespace SearchAGram {
  
  class Config {
  public:
    static void populateFromFile (const std::string& filename);
    
    template<typename T>
    using optional = typename boost::optional<T>;
   
    template<typename T>
    static T get (const boost::property_tree::ptree::path_type& path) {
      return ptree_.get<T> (path);
    }

    template<typename T>
    static optional<T> get_optional (
        const boost::property_tree::ptree::path_type& path) {
      return ptree_.get_optional<T> (path);
    }

  private:
    static boost::property_tree::ptree ptree_;
  };

}

#endif /* SEARCHAGRAM_CONFIG_H_ */

/* vim: set ts=2 sw=2 et: */
