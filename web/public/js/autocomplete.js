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

jQuery (document).ready (function () {
  // Populates the filters list.
  jQuery.ajax ('/get_filters', {
    error: function (jqXHR, textStatus, errorThrown) {
      // TODO: show some error dialog.
      console.error ('Unable to /get_filters');
    },
    success: function (data, textStatus, jqXHR) {
      if (!data.results || data.results.length == 0) {
        // TODO: show some error dialog.
        console.error ('Unable to /get_filters (invalid data)');
        return;
      }

      // Insert an empty element.
      jQuery ('#filter').empty ();
      var results = data.results;
      for (var i = 0; i < results.length; i++) {
        var result = results[i];
        if (result.length == 0) {
          continue;
        }

        var option = jQuery (document.createElement ('option'))
          .attr ('value', result)
          .append (result);
        jQuery ('#filter').append (option);
      }
    },
  });

  // Provides autocompletion suggestions for users.
  jQuery ('#user').bind ('input', function (input_event) {
    var user = jQuery (this).val ();
    var user_field = this;

    jQuery.ajax ('/autocomplete/users', {
      error: function (jqXHR, textStatus, errorThrown) {
        // TODO: show some error dialog.
      },
      success: function (data, textStatus, jqXHR) {
        if (!data.results || data.results.length == 0) {
          jQuery ('#users-completion-list').hide ();
          return;
        }
        
        // We haz data. Populate the autocompletion list.
        var results = data.results;
        jQuery ('#users-completion-list').empty ();
        for (var i = 0; i < results.length; i++) {
          var list_item = jQuery (document.createElement ('li'))
            .append (jQuery (document.createElement ('a'))
              .attr ('href', '#')
              .append (jQuery (document.createElement ('img'))
                .attr ('src', results[i].profile_picture)
                .addClass ('profile_picture')
                .bind ('error', function (error_event) {
                  this.src = "assets/profile-picture-missing.png";
                }))
              .append (jQuery (document.createElement ('span'))
                .append (results[i].username)
                .addClass ('username'))
              .click (function (click_event) {
                var username = jQuery (this).find ('.username').html ();
                jQuery (user_field).val (username);
                jQuery ('#users-completion-list').hide ();
                click_event.preventDefault ();
              }));
          jQuery ('#users-completion-list').append (list_item);
        }
        
        // Set the position of the list.
        var position = jQuery (user_field).offset ();
        jQuery ('#users-completion').css ({
          'position': 'absolute',
          'top': position.top + jQuery(user_field).height () + 15 + 'px',
          'left': position.left + 'px'
        });

        jQuery ('#users-completion-list').show ();
      },
      data: { "query": user }
    });
  });

  jQuery (document).click (function (click_event) {
    jQuery ('#users-completion-list').hide ();
  });

  jQuery (window).resize (function () {
    jQuery ('#users-completion-list').hide ();
  });
});

/* vim: set ts=2 sw=2 et: */
