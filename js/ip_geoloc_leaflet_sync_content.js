(function ($) {

  var LEAFLET_SYNC_CONTENT_TO_MARKER = 1 << 1;
  var LEAFLET_SYNC_MARKER_TO_CONTENT = 1 << 2;
  var LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP = 1 << 3;

  var SYNCED_MARKER_HOVER  = 'synced-marker-hover';
  var SYNCED_CONTENT_HOVER = 'synced-content-hover';

  var lastMarkerSelector = null;

  $(document).bind('leaflet.feature', function(e, lFeature, feature) {

    // lFeature is the marker, polygon, linestring... just created on the map.
    // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leafet.inc
    var contentSelector = ".sync-id-" + feature.feature_id;

    if ((feature.flags & LEAFLET_SYNC_CONTENT_TO_MARKER) && feature.feature_id) {
      lFeature.on('mouseover', function(e) {
        $(contentSelector).addClass(SYNCED_MARKER_HOVER);
      });
      lFeature.on('mouseout', function(e) {
        $(contentSelector).removeClass(SYNCED_MARKER_HOVER);
      });
    }

    if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT) && feature.tooltip) {
      // Test for "\n" inserted by region differentiator, if selected.
      var nl = feature.tooltip.indexOf("\n");
      var tooltip = (nl < 0) ? feature.tooltip : feature.tooltip.substring(0, nl);

      // Can't seem to set an id on the marker image, so abusing tooltip
      // to identify marker. title attribute either appears on img or on the
      // parent-div of img.
      // If tooltip is a number compare "whole word", otherwise "starts-with"
      var markerSelector = isNaN(tooltip)
        ? ".leaflet-marker-pane *[title^='" + tooltip + "']"  // starts-with
        : ".leaflet-marker-pane *[title~='" + tooltip + "']"; // whole-word

      // Using bind() as core's jQuery is old and does not support on()
      $(contentSelector).bind('mouseover', function(e) {
        if (markerSelector !== lastMarkerSelector) {
          
          $(lastMarkerSelector).removeClass(SYNCED_CONTENT_HOVER);
          $(markerSelector).addClass(SYNCED_CONTENT_HOVER);
          lastMarkerSelector = markerSelector;

          if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP) && feature.tooltip) {
            lFeature._popup.options.offset.y -= 19;
            lFeature.openPopup();
            lFeature._popup.options.offset.y += 19;
          }
        }
      });
      //$(contentSelector).bind('mouseout', function(e) {
      //});
    }
  });

  $(document).ready(function() {
    // On map mouse: revert all altered markers and close all popus.
    for (var i = 0; i < Drupal.settings.leaflet.length; i++) {
      Drupal.settings.leaflet[i].lMap.on('mouseout', function(e) {
        e.target.closePopup();
        $(lastMarkerSelector).removeClass(SYNCED_CONTENT_HOVER);
      });
    }
  });

})(jQuery);
