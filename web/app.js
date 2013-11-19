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

// User application configuration
var config = require ('./config')

// Module dependencies
var express = require ('express')
var http = require ('http')
var path = require ('path')

var app = express ();

// Application configuration: all environments
app.set ('views', __dirname + '/views')
app.set ('view engine', 'jade')
app.use (express.static (path.join (__dirname, 'public')));
app.use (express.favicon ());

app.use (express.logger ('dev'));
app.use (express.bodyParser ());
app.use (express.methodOverride ());

app.use (app.router);

// Application configuration: development
if ('development' == config.env) {
  app.locals.pretty = true;
  app.use (express.errorHandler ());
}

// Routes configuration.
var routes = require ('./routes');

app.get ('/search', routes.search);
app.get ('/search', routes.image_search);

app.get ('/get_filters', routes.get_filters);
app.get ('/autocomplete/tags', routes.autocomplete_tags);
app.get ('/autocomplete/users', routes.autocomplete_users);
app.get ('/test', routes.test);

// Create server.
http.createServer (app).listen (
  config.web.http.port,
  config.web.http.address,
  function () {
    console.log ('Search-a-gram listening on port ' + config.web.http.port);
  }
);

// vim: set ts=2 sw=2 et:
