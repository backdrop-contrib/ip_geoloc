(function ($) {

  var LEAFLET_SYNC_CONTENT_TO_MARKER = 1 << 1;
  var LEAFLET_SYNC_MARKER_TO_CONTENT = 1 << 2;
  var LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP = 1 << 3;
//var LEALFET_SYNC_REVERT_LAST_MARKER_ON_MAP_OUT = 1 << 4;

  var SYNCED_CONTENT_HOVER = 'synced-content-hover';
  var SYNCED_MARKER_HOVER  = 'synced-marker-hover';
  var SYNCED_MARKER_HIDDEN = 'leaflet-marker-hidden';

  var lastMarker = null;
  var markersOriginallyVisible = [];

  $(document).bind('leaflet.feature', function(event, marker, feature) {

    // marker is the feature just added to the map, it could be a polygon too.
    // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc

    if (!feature.feature_id) {
      return;
    }

    var contentSelector = ".sync-id-" + feature.feature_id;
    if (feature.flags & LEAFLET_SYNC_CONTENT_TO_MARKER) {
      marker.on('mouseover', function(event) {
        $(contentSelector).addClass(SYNCED_MARKER_HOVER);
      });
      marker.on('mouseout', function(event) {
        $(contentSelector).removeClass(SYNCED_MARKER_HOVER);
      });
    }

    if (feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT) {

      marker.on('popupclose', function(event) {
        var element = marker._icon ? marker._icon : marker._container;
        if (element) {
          L.DomUtil.removeClass(element, SYNCED_CONTENT_HOVER);
          // On popupclose hide the marker, iff it wasn't visible to begin with.
          if (!markersOriginallyVisible[event.target._leaflet_id]) {
            L.DomUtil.addClass(event.target._icon, SYNCED_MARKER_HIDDEN);
          }
        }
        lastMarker = null;
      });

      // Using bind() as D7 core's jQuery is old and does not support on()
      $(contentSelector).bind('mouseover', function(event) {

        if (typeof(markersOriginallyVisible[marker._leaflet_id]) === 'undefined') {
          markersOriginallyVisible[marker._leaflet_id] = (marker._map ? true : false);
        }
        if (marker !== lastMarker) {

          if (lastMarker && !markersOriginallyVisible[lastMarker._leaflet_id]) {
            // Hide the previously highlighted marker, before highlighting next.
            if (lastMarker._icon) {
              L.DomUtil.addClass(lastMarker._icon, SYNCED_MARKER_HIDDEN);
            }
            lastMarker.closePopup();
          }
          var isVisible = marker._map;
          if (!isVisible) {
            // If marker doesn't have a map, but a (grand)parent does, use that.
            for (var parent = marker; parent; parent = parent.__parent) {
              if (parent._map) break;
            }
            if (parent && parent._map) {
              marker.addTo(parent._map);
            }
          }
          var element = marker._icon ? marker._icon : marker._container;
          if (isVisible) {
            // Marker with icon: make it visible, in case it was invisible.
            L.DomUtil.removeClass(element, SYNCED_MARKER_HIDDEN);
          }

          // This does not work in Chrome or Safari, but works in Firefox.
          if (marker._bringToFront) {
            marker._bringToFront();
          }

          if (element) {
            // Now that it is visible, add to the marker the special CSS class.
            L.DomUtil.addClass(element, SYNCED_CONTENT_HOVER);
          }

          if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP)) {
            // Popup requested
            marker._popup.options.offset.y -= 19;
            marker.openPopup(); // 
            marker._popup.options.offset.y += 19;
          }
          else if (lastMarker) {
            // If LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP is set, this is
            // automatically taken care of by the 'popupclose' event handler
            // above. This in turn is triggered by a an openPopup() call on
            // another marker.
            var lastElement = lastMarker._icon ? lastMarker._icon : lastMarker._container;
            L.DomUtil.removeClass(lastElement, SYNCED_CONTENT_HOVER);
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
