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

exports.test = function (req, res) {
  res.render ('index', { title: 'Home' });

  var config = require ('../config');
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });
  socket.connect (config.backend.port, function () {
    var fs = require ('fs');
    var file = fs.readFileSync ('../test/images/b58/b58a6912cc6611e2971f22000a1f8c25_7.jpg', 'base64');
    
    var str = JSON.stringify ({"action": "search", "query": file});
    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);

      console.log (data);
    });
  });
};

exports.get_filters = function (req, res) {
  var config = require ('../config');
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });

  socket.connect (config.backend.port, function () {
    var str = JSON.stringify ({
      "action": "get_filters"
    });

    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);
      res.json (data);
    });
  });
};

exports.autocomplete_tags = function (req, res) {
  var config = require ('../config');
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });

  socket.connect (config.backend.port, function () {
    var str = JSON.stringify ({
      "action": "autocomplete_hashtags",
      "query": req.query.query
    });

    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);
      res.json (data);
    });
  });
}

exports.autocomplete_users = function (req, res) {
  var config = require ('../config');
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });

  socket.connect (config.backend.port, function () {
    var str = JSON.stringify ({
      "action": "autocomplete_users",
      "query": req.query.query
    });

    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);
      res.json (data);
    });
  });
}

exports.search = function (req, res) {
  var config = require ('../config');
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });

  socket.connect (config.backend.port, function () {
    // Preprocess some search parameters.
    var hashtags = req.query.hashtags
      .split (",")
      .filter (function (value, index, hashtags) {
        return value;
      })
      .map (function (value, index, hashtags) {
        value = value.trim ();
        if (value.substring (0, 1) == "#") {
          value = value.substring (1);
        }

        return value;
      });

    var query = {
      "action": "search",
      "type": req.query.type,
      "username": req.query.username,
      "hashtags": hashtags,
      "filter": req.query.filter,
      "date_interval": req.query.date_interval,
      "likes_interval": req.query.likes_interval,
      "comments_interval": req.query.comments_interval
    };

    if (req.query.likes && req.query.likes.length > 0) {
      query.likes = parseInt (req.query.likes);
    } else {
      query.likes = -1;
    }

    if (req.query.comments && req.query.comments.length > 0) {
      query.comments = parseInt (req.query.comments);
    } else {
      query.comments = -1;
    }

    if (req.query.date.length > 0) {
      query.date = Math.floor (new Date (req.query.date).getTime () / 1000);
    } else {
      query.date = Math.floor (new Date ().getTime () / 1000);
    }

    var str = JSON.stringify (query);

    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);
      res.json (data);
    });
  });
}

exports.image_search = function (req, res) {
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });
  var fs = require ('fs');
  var file = fs.readFileSync (req.files.image.path, 'base64');

  var startTime = process.hrtime ();
  socket.connect (9192, function () {
    var str = JSON.stringify ({"action": "image_search", "query": file});
    socket.write (str);
    socket.write ('\n');

    var results = null;
    socket.on ('data', function (data) {
      if (results == null) {
        results = data;
      } else {
        results = Buffer.concat ([results, data]);
      }
    });

    socket.on ('close', function () {
      var data = results.toString ();
      data = JSON.parse (data);
      console.log (data);
      res.json (data);

      var elapsed = process.hrtime (startTime);
    });
  });
};

// vim: set ts=2 sw=2 et:
