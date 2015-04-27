L.sync = {};

(function ($) {

  var LEAFLET_SYNC_CONTENT_TO_MARKER = 1 << 1;
  var LEAFLET_SYNC_MARKER_TO_CONTENT = 1 << 2;
  var LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP = 1 << 3;

  var SYNCED_CONTENT_HOVER = 'synced-content-hover';
  var SYNCED_MARKER_HOVER  = 'synced-marker-hover';
  var SYNCED_MARKER_HIDDEN = 'leaflet-marker-hidden';

  var lastMarker = null;
  var markersOriginallyVisible = [];

  L.sync.contentToMarker = function(contentSelector, marker, className) {
    marker.on('mouseover', function(event) {
      $(contentSelector).addClass(className);
    });
    marker.on('mouseout', function(event) {
      $(contentSelector).removeClass(className);
    });
  };

  L.sync.markerToContent = function(contentSelector, marker, feature) {

    marker.on('popupclose', function(event) {
      L.sync.removeClass(marker, SYNCED_CONTENT_HOVER);
      // On popupclose hide the marker, iff it wasn't visible to begin with.
      if (!markersOriginallyVisible[event.target._leaflet_id]) {
        L.sync.addClass(marker, SYNCED_MARKER_HIDDEN);
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
          L.sync.addClass(lastMarker, SYNCED_MARKER_HIDDEN);
          lastMarker.closePopup();
        }
        var isVisible = marker._map;
        if (!isVisible) {
          // If marker doesn't have a map, but an ancestor does, use that.
          for (var parent = marker; parent; parent = parent.__parent) {
            if (parent._map) break;
          }
          if (parent && parent._map) {
            marker.addTo(parent._map);
          }
        }
        if (isVisible) {
          // Marker with icon: make it visible, in case it was invisible.
          L.sync.removeClass(marker, SYNCED_MARKER_HIDDEN);
        }
        // This doesn't work in Chrome/Safari, but sometimes in Firefox.
        if (marker._bringToFront) {
          marker._bringToFront();
        }
        // Now that it is visible, add to the marker the special CSS class.
        L.sync.addClass(marker, SYNCED_CONTENT_HOVER);

        if ((feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP)) {
          // Popup requested
          if (marker._icon) marker._popup.options.offset.y -= 19;
          marker.openPopup();
          if (marker._icon) marker._popup.options.offset.y += 19;
        }
        else if (lastMarker) {
          // If LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP is set, this is
          // automatically taken care of by the 'popupclose' event handler
          // above. This in turn is triggered by an openPopup() call on
          // another marker.
          L.sync.removeClass(lastMarker, SYNCED_CONTENT_HOVER);
        }
        lastMarker = marker;
      }
    });
  };

  $(document).bind('leaflet.feature', function(event, marker, feature) {

    // marker is the feature just added to the map, it could be a polygon too.
    // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
    if (!feature.feature_id) {
      return;
    }

    marker.on('mouseover', function(event) {
      markersOriginallyVisible[event.target._leaflet_id] = true;
    });
    marker.on('popupopen', function(event) {
      markersOriginallyVisible[event.target._leaflet_id] = true;
    });

    var contentSelector = ".sync-id-" + feature.feature_id;

    if (feature.flags & LEAFLET_SYNC_CONTENT_TO_MARKER) {
      L.sync.contentToMarker(contentSelector, marker, SYNCED_MARKER_HOVER);
    }

    if (feature.flags & LEAFLET_SYNC_MARKER_TO_CONTENT) {
      L.sync.markerToContent(contentSelector, marker, feature);
    }
  });

  $(document).bind('leaflet.map', function(event, map, lMap) {

    if (map.settings.revertLastMarkerOnMapOut) {
      // On map mouse hover out: close all popus and revert synced marker style.
      lMap.on('mouseout', function(event) {
        event.target.closePopup();
        if (lastMarker) {
          L.sync.removeClass(lastMarker, SYNCED_CONTENT_HOVER);
          if (!markersOriginallyVisible[lastMarker._leaflet_id]) {
            L.sync.addClass(lastMarker, SYNCED_MARKER_HIDDEN);
          }
        }
        lastMarker = null;
      });
    }
  });

})(jQuery);

L.sync.getMarkerElements = function(marker) {
  var elements = [];
  if (marker._icon) {
    elements.push(marker._icon);
  }
  if (marker._container) {
    elements.push(marker._container);
  }
  if (marker._layers) {
    for (var key in marker._layers) elements.push(marker._layers[key]._container);
  }
  return elements;
};

L.sync.addClass = function(marker, className) {
  var elements = L.sync.getMarkerElements(marker);
  for (var i = 0; i < elements.length; i++) {
    L.DomUtil.addClass(elements[i], className);
  }
};

L.sync.removeClass = function(marker, className) {
  var elements = L.sync.getMarkerElements(marker);
  for (var i = 0; i < elements.length; i++) {
    L.DomUtil.removeClass(elements[i], className);
  }
};