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
  // 180px height for basic
  // 415px for advanced
  
  /**
   * Actions for "Basic Search" and "Advanced Search".
   */
  jQuery ('#advanced-button').click (function (e) {
    jQuery ('#image').addClass ('expanded-dropbox');
    jQuery ('#advanced-search')
      .show ()
      .addClass ('advanced-search-shown');
    jQuery (this).hide ();
    jQuery ('#basic-button').show ();
    e.preventDefault ();
  });
  
  jQuery ('#basic-button').click (function (e) {
    var button = this;
    jQuery ('#image').removeClass ('expanded-dropbox');
    jQuery ('#advanced-search')
      .removeClass ('advanced-search-shown');
      
    setTimeout (function () {
      button.hide ();
    }, 1000);
    
    jQuery (this).hide ();
    jQuery ('#advanced-button').show ();
    e.preventDefault ();
  });

  /**
   * Search result box.
   */
  var createSearchResultItem = function createSearchResultItem (result) {
    var result_item = jQuery (document.createElement ('div'))
      .addClass ('col-sm-3')
      .css ('min-height', '275px')
      .css ('padding-bottom', '20px')
      .append (jQuery (document.createElement ('div'))
        .addClass ('placeholder')
        .append (jQuery (document.createElement ('a'))
          .attr ('href', result.link)
          .attr ('target', '_blank')
          .append (jQuery (document.createElement ('img'))
            .attr ('src', result.url)
            .bind ('error', function (load_error) {
              this.src = "assets/profile-picture-missing.png";
              jQuery (this).css ('height', "207px");
              jQuery (this).css ('width', "207px");
            })))
        .append (jQuery (document.createElement ('p'))
          .append (jQuery (document.createElement ('span'))
            .addClass ('glyphicon')
            .addClass ('glyphicon-heart'))
          .append (" ")
          .append (result.likes_count)
          .append (jQuery (document.createElement ('span'))
            .addClass ('glyphicon')
            .addClass ('glyphicon-comment')
            .css ('margin-left', '20px'))
          .append (" ")
          .append (result.comments_count)));

    return result_item;
  };
 
  /**
   * Text search implementation.
   */
  jQuery ('#text-search').submit (function (submit_event) {
    // Show the found query.
    if (jQuery ("#user").val ().length == 0) {
      jQuery ('#found-username').empty ().append ("everyone");
    } else {
      jQuery ('#found-username').empty ().append (jQuery ("#user").val ());
    }

    if (jQuery ('#hashtag').val ().length == 0) {
      jQuery ('#found-hashtags').empty ().append ("none");
    } else {
      var hashtags = jQuery ('#hashtag').val ()
        .split (",")
        .filter (function (value, index, array) {
          return value;
        })
        .map (function (value, index, array) {
          if (value.substring (0, 1) != "#") {
            return "#" + value;
          }

          return value;
        });

      jQuery ('#found-hashtags').empty ().append (hashtags.join (", "));
    }

    if (jQuery ('#filter').val ().length == 0) {
      jQuery ('#found-filter').empty ().append ("filter");
    } else {
      jQuery ('#found-filter').empty ().append (jQuery ('#filter').val ());
    }

    jQuery.ajax ('/search?' + jQuery ('#text-search').serialize (), {
      error: function (jqXHR, textStatus, errorThrown) {
        // TODO: show some error dialog.
        jQuery ('#no-results-found').show ();
        jQuery ('#results-found').hide ();
        console.error ('Unable to /search');
      },
      complete: function (jqXHR, textStatus) {
        jQuery ('#section-results').show ();
        jQuery.scrollTo ('#section-results', 1000);
      },
      success: function (data, textStatus, jqXHR) {
        jQuery ('#results').empty ();

        if (!data.results || data.results.length == 0) {
          // TODO: show some error dialog.
          jQuery ('.no-results-found').show ();
          jQuery ('.results-found').hide ();
          return;
        }

        // Insert an empty element.
        var results = data.results;
        for (var i = 0; i < results.length; i++) {
          var result = results[i];
          var result_item = createSearchResultItem (result);
          jQuery ('#results').append (result_item);
        }

        jQuery ('.results-found').show ();
        jQuery ('.no-results-found').hide ();
      },
    });

    submit_event.preventDefault ();
  });

  /**
   * Image search implementation.
   */
  // Make clicking the upload area open the upload dialog.
  jQuery ('#image').click (function () {
    jQuery ('#image-upload').click ();
  });

  // Upload the file on change.
  jQuery ('#image-upload').change (function (change_event) {
    var form_data = new FormData (document.getElementById ('image-search-form'));
    jQuery.ajax ({
      url: '/image_search',
      type: 'POST',
      cache: false,
      contentType: false,
      data: form_data,
      processData: false,
      success: function (data, textStatus, jqXHR) {
        jQuery ('#results').empty ();

        if (!data.results || data.results.length == 0) {
          // TODO: show some error dialog.
          jQuery ('.no-results-found').show ();
          jQuery ('.results-found').hide ();
          return;
        }

        // Insert an empty element.
        var results = data.results;
        for (var i = 0; i < results.length; i++) {
          var result = results[i];
          var result_item = createSearchResultItem (result);
          jQuery ('#results').append (result_item);
        }

        jQuery ('.results-found').show ();
        jQuery ('.no-results-found').hide ();
      },
      error: function (jqXHR, textStatus, errorThrown) {
        // TODO: show some error dialog.
        jQuery ('#no-results-found').show ();
        jQuery ('#results-found').hide ();
        console.error ('Unable to /image_search');
      },
      complete: function (jqXHR, textStatus) {
        jQuery ('#section-results').show ();
        jQuery.scrollTo ('#section-results', 1000);
      }
    });
  });

  /**
   * Should revert when logo is clicked.
   */
  jQuery ('#logo').click (function (click_event) {
  });

  // Enable smooth scrolling on links.
  jQuery.localScroll ();
});

/* vim: set ts=2 sw=2 et: */
