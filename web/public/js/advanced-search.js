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
//180px height for basic
//415px for advanced

    jQuery ('#advanced-button').click (function (e) {
      jQuery ('#image').addClass ('expanded-upload-box');
      jQuery ('#advanced-search')
        .show ()
        .addClass ('advanced-search-shown');
      jQuery (this).hide ();
      jQuery ('#basic-button').show ();
      e.preventDefault ();
    });
    
    jQuery ('#basic-button').click (function (e) {
      var button = this;
      jQuery ('#image').removeClass ('expanded-upload-box');
      jQuery ('#advanced-search')
        .removeClass ('advanced-search-shown');
        
      setTimeout (function () {
        button.hide ();
      }, 1000);
      
      jQuery (this).hide ();
      jQuery ('#advanced-button').show ();
      e.preventDefault ();
    });
    
    var advanced = function () {  
        jQuery ('#image').css (
            'height', '415px'
        );
    };

    jQuery.localScroll ();
});

