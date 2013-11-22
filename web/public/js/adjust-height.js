jQuery (document).ready (function () {
    var adjustHeight = function () {
        jQuery ('section#section-search').css (
            'height', (jQuery (window).height () - 50) + 'px'
        );
    };
    
    // Adjust the height of the grey part when the page is loaded.
    adjustHeight ();
    
    // Adjust the height of the grey part when the window is resized.
    jQuery (window).resize (adjustHeight);
});

