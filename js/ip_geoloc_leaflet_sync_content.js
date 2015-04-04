(function ($) {

  var LEAFLET_SYNC_CONTENT_TO_MARKER = 1 << 1;
  var LEAFLET_SYNC_MARKER_TO_CONTENT = 1 << 2;
  var LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP = 1 << 3;
  var lastMarkerSelector = null;

  $(document).bind('leaflet.feature', function(e, lFeature, feature) {

    // lFeature is the marker, polygon, linestring... just created on the map.
    // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leafet.inc
    var contentSelector = ".node-" + feature.feature_id;

    if ((feature.flags & LEAFLET_SYNC_CONTENT_TO_MARKER) && feature.feature_id) {
      lFeature.on('mouseover', function(e) {
        $(contentSelector).addClass('synced-marker-hover');
      });
      lFeature.on('mouseout', function(e) {
        $(contentSelector).removeClass('synced-marker-hover');
      });
    }

    if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT) && feature.tooltip) {
      var nl = feature.tooltip.indexOf("\n");
      var tooltip = (nl < 0) ? feature.tooltip : feature.tooltip.substring(0, nl);
      // Can't seem to set an id on the marker image, so abusing tooltip
      // to identify marker image. title attribute either appears on img or
      // on parent-div of img.
      // If tooltip is a number compare "whole word", otherwise "starts-with"
      var markerSelector = isNaN(tooltip)
        ? "*[title^='" + tooltip + "']" : "*[title~='" + tooltip + "']";
      // Using bind() as core's jQuery is old and does not support on()
      $(contentSelector).bind('mouseover', function(e) {
        if (lastMarkerSelector !== markerSelector) {
          if (lastMarkerSelector) {
            $(lastMarkerSelector).removeClass('synced-content-hover');
          }
          $(markerSelector).addClass('synced-content-hover');
          lastMarkerSelector = markerSelector;
          if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP) && feature.tooltip) {
            lFeature._popup.options.offset.y -= 19;
            lFeature.openPopup();
            lFeature._popup.options.offset.y += 19;
          }
        }
      });
      $(contentSelector).bind('mouseout', function(e) {
         //$(lastMarkerSelector).removeClass('synced-content-hover');
      });
    }
  });

  $(document).ready(function() {
    // For all maps: close all popus on map mouseout.
    for (var i = 0; i < Drupal.settings.leaflet.length; i++) {
      var map = Drupal.settings.leaflet[i].lMap;
      map.on('mouseout', function(e) {
        e.target.closePopup();
      });
    }
  });

})(jQuery);
