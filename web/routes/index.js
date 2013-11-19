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

exports.get_image = function (req, res) {
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });

  setTimeout (function () {
    socket.connect (9192, function () {
      var str = JSON.stringify ({
        "action": "get_image",
        "filename": req.params.filename
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
        
        if (data.status == 0) {
          res.type (data.mime);
          res.send (new Buffer (data.image, 'base64'));

        } else {
          res.send (500, data.description);
        }
      });
    });
  }, 1000);
};

exports.search = function (req, res) {
  var net = require ('net');
  var socket = new net.Socket ({type: 'tcp4' });
  var fs = require ('fs');
  var file = fs.readFileSync (req.files.image.path, 'base64');

  var startTime = process.hrtime ();
  socket.connect (9192, function () {
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

      data.results = data.results.slice (0, 20);

      var elapsed = process.hrtime (startTime);
      res.render ('results', {
        title: 'Search Results',
        results: data.results,
        elapsed: elapsed
      });
    });
  });
};

// vim: set ts=2 sw=2 et:
