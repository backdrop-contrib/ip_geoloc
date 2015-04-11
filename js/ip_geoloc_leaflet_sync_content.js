(function ($) {

  var LEAFLET_SYNC_CONTENT_TO_MARKER = 1 << 1;
  var LEAFLET_SYNC_MARKER_TO_CONTENT = 1 << 2;
  var LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP = 1 << 3;
  var LEALFET_SYNC_REVERT_LAST_MARKER_ON_MAP_OUT = 1 << 4;

  var SYNCED_CONTENT_HOVER = 'synced-content-hover';
  var SYNCED_MARKER_HOVER  = 'synced-marker-hover';
  var SYNCED_MARKER_HIDDEN = 'leaflet-marker-hidden';

  var lastMarker = null;
  //var markerWasOnMap = false;

  $(document).bind('leaflet.feature', function(event, marker, feature) {

    marker.on('popupclose', function(event) {
      if (event.target._icon) {
        L.DomUtil.removeClass(event.target._icon, SYNCED_CONTENT_HOVER);
      }
    });

    // marker is the marker/polygon/linestring... just created on the map.
    // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc

    var contentSelector = ".sync-id-" + feature.feature_id;
    if ((feature.flags & LEAFLET_SYNC_CONTENT_TO_MARKER) && feature.feature_id) {
      marker.on('mouseover', function(event) {
        $(contentSelector).addClass(SYNCED_MARKER_HOVER);
      });
      marker.on('mouseout', function(event) {
        $(contentSelector).removeClass(SYNCED_MARKER_HOVER);
      });
    }

    // Only do this for markers that have icons (i.e. not for clusters).
    if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT) && !marker._icon) {
      // Using bind() as D7 core's jQuery is old and does not support on()
      $(contentSelector).bind('mouseover', function(event) {

        if (marker !== lastMarker) {

          if (lastMarker && lastMarker._icon) {
            // Hide the previously highlighted marker, before highlighting next.
            L.DomUtil.addClass(lastMarker._icon, SYNCED_MARKER_HIDDEN);
          }

          if (marker._map) {
            // Make existing marker visible, in case it was invisible.
            // markerWasOnMap = true;
            L.DomUtil.removeClass(marker._icon, SYNCED_MARKER_HIDDEN);
          }
          else {
            // markerWasOnMap = false;
            // If marker doesn't have a map, but a (grand)parent does, use that.
            for (var parent = marker; parent; parent = parent.__parent) {
              if (parent._map) break;
            }
            if (parent && parent._map) {
              marker.addTo(parent._map);
            }
          }
          if (marker._icon) {
            // Now that it is visible, add to the marker the special CSS class.
            L.DomUtil.addClass(marker._icon, SYNCED_CONTENT_HOVER);
            // This does not work in Chrome or Safari, but works fine in Firefox.
            marker._bringToFront();
          }

          if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP)) {
            // Popup requested
            marker._popup.options.offset.y -= 19;
            marker.openPopup(); // 
            marker._popup.options.offset.y += 19;
          }
          else if (lastMarker && lastMarker._icon) {
            // If LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP is set, this is
            // automatically taken care of by the 'popupclose' event handler
            // above. This in turn is triggered by a an openPopup() call on
            // another marker.
            L.DomUtil.removeClass(lastMarker._icon, SYNCED_CONTENT_HOVER);
          }
          lastMarker = marker;
        }
      });
    }
  });
/*
  $(document).bind('leaflet.map', function(event, map, lMap) {
    // On map mouse hover out: close all popus, reverting synced marker style.
    lMap.on('mouseout', function(event) {
      event.target.closePopup();
      L.DomUtil.removeClass(lastMarker._icon, SYNCED_CONTENT_HOVER);
    });
  });
*/
})(jQuery);
