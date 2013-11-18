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

// Do not change these directives.
var config = {};
config.web = {};
config.web.http = {};
config.backend = {};

/**
 * Web server configuration
 */

// Listening address for HTTP.
// Change this to an interface address only if you want the server to listen
// on a particular interface.
config.web.http.address = '0.0.0.0';

// Listening port for HTTP.
// The default (port 80) should work fine in most cases, but you need root
// privileges to run the server.
config.web.http.port = 8080;

// Application environment.
config.env = 'development';

/**
 * Search-a-gram backend configuration
 */

// Port for the search server.
config.backend.port = 9192;

/**
 * No need to edit anything below here.
 */

module.exports = config;

// vim: set ts=2 sw=2 et:
