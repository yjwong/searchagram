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

});