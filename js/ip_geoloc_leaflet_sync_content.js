
L.Sync = {
  SYNC_CONTENT_TO_MARKER: 1 << 1,
  SYNC_MARKER_TO_CONTENT: 1 << 2,
  SYNC_MARKER_TO_CONTENT_WITH_POPUP: 1 << 3,

  SYNCED_CONTENT_HOVER: 'synced-content-hover',
  SYNCED_MARKER_HOVER : 'synced-marker-hover',
  SYNCED_MARKER_HIDDEN: 'leaflet-marker-hidden',

  map: null,
  visibleMarkers: [],
  lastMarker: null,

  isVisible: function(marker) {
    this.ensureVisiblesAreRecorded();
    return marker && marker._leaflet_id && this.visibleMarkers[marker._leaflet_id];
  },

  ensureVisiblesAreRecorded: function() {
    if (this.visibleMarkers.length) {
      return;
    }
    var allMarkers = [];
    // There's only one _topClusterLevel, find it.
    for (var leaflet_id in this.map._layers) {
      if (this.map._layers[leaflet_id]._topClusterLevel) {
        var topClusterLevel = this.map._layers[leaflet_id]._topClusterLevel;
        allMarkers = topClusterLevel.getAllChildMarkers();
        break;
      }
    }
    for (var i in allMarkers) {
      if (allMarkers[i]._leaflet_id) {
        var id = allMarkers[i]._leaflet_id;
        this.visibleMarkers[id] = (allMarkers[i]._map ? true : false);
      }
    }
  },
 
  invalidateVisibles: function() {
    this.visibleMarkers = [];
  },

  contentToMarker: function(contentSelector, marker, className) {
    marker.on('mouseover', function(event) {
      jQuery(contentSelector).addClass(className);
    });
    marker.on('mouseout', function(event) {
      jQuery(contentSelector).removeClass(className);
    });
  },

  markerToContent: function(contentSelector, marker, feature) {

    marker.on('popupclose', function(event) {
      L.Sync.removeClass(marker, L.Sync.SYNCED_CONTENT_HOVER);
      // On popupclose hide the marker, iff it wasn't visible to begin with.
      if (!L.Sync.isVisible(marker)) {
        L.Sync.addClass(marker, L.Sync.SYNCED_MARKER_HIDDEN);
      }
      L.Sync.lastMarker = null;
    });

    // Using bind() as D7 core's jQuery is old and does not support on()
    jQuery(contentSelector).bind('mouseover', function(event) {

      if (marker === L.Sync.lastMarker) {
        return;
      }
      L.Sync.ensureVisiblesAreRecorded();

      if (L.Sync.lastMarker && !L.Sync.isVisible(L.Sync.lastMarker)) {
        // Hide the previously highlighted marker, before highlighting next.
        L.Sync.addClass(L.Sync.lastMarker, L.Sync.SYNCED_MARKER_HIDDEN);
        L.Sync.lastMarker.closePopup();
      }
      if (!marker._map) {
        marker.addTo(L.Sync.map);
      }
      if (marker._map) {
        // This doesn't work in Chrome/Safari, but sometimes in Firefox.
        marker._bringToFront();
      }
      // Make marker visible, in case it was invisible.
      L.Sync.removeClass(marker, L.Sync.SYNCED_MARKER_HIDDEN);

      // Now that it is visible, add to the marker the special CSS class.
      L.Sync.addClass(marker, L.Sync.SYNCED_CONTENT_HOVER);

      if ((feature.flags & L.Sync.SYNC_MARKER_TO_CONTENT_WITH_POPUP)) {
        // Popup requested
        if (marker._icon) marker._popup.options.offset.y -= 19;
        marker.openPopup();
        if (marker._icon) marker._popup.options.offset.y += 19;
      }
      else if (L.Sync.lastMarker) {
        // If LEAFLET_SYNC_MARKER_TO_CONTENT_WITH_POPUP is set, this is
        // automatically taken care of by the 'popupclose' event handler
        // above. This in turn is triggered by an openPopup() call on
        // another marker.
        L.Sync.removeClass(L.Sync.lastMarker, L.Sync.SYNCED_CONTENT_HOVER);
      }
      L.Sync.lastMarker = marker;
    });
  },

  addClass:function(marker, className) {
    var elements = this.getMarkerElements(marker);
    for (var i = 0; i < elements.length; i++) {
      L.DomUtil.addClass(elements[i], className);
    }
  },

  removeClass: function(marker, className) {
    var elements = this.getMarkerElements(marker);
    for (var i = 0; i < elements.length; i++) {
      L.DomUtil.removeClass(elements[i], className);
    }
  },

  getMarkerElements: function(marker) {
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
  }
};

jQuery(document).bind('leaflet.map', function(event, map, lMap) {
  // Todo: create instance of L.Sync
  // At this point lMap._layers contains visitor location and clusters.
  lMap.sync = L.Sync;
  L.Sync.map = lMap;

  // When zoom level is changed, invalidate the list of visible markers.
  lMap.on('zoomend', function(event) {
    event.target.sync.invalidateVisibles();
  });

  if (map.settings.revertLastMarkerOnMapOut) {
    // On map mouse hover out: close all popus and revert synced marker style.
    lMap.on('mouseout', function(event) {
      var map = event.target;
      map.closePopup();
      if (map.sync.lastMarker) {
        L.Sync.removeClass(map.sync.lastMarker, L.Sync.SYNCED_CONTENT_HOVER);
        if (!map.sync.isVisible(map.sync.lastMarker)) {
          L.Sync.addClass(map.sync.lastMarker, L.Sync.SYNCED_MARKER_HIDDEN);
        }
      }
      map.sync.lastMarker = null;
    });
  }
});

jQuery(document).bind('leaflet.feature', function(event, marker, feature) {
  // marker is the feature just added to the map, it could be a polygon too.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  if (!feature.feature_id) {
    return;
  }
  // A CSS class, not an ID as multiple markers may be attached to same node.
  var contentSelector = ".sync-id-" + feature.feature_id;

  if (feature.flags & L.Sync.SYNC_CONTENT_TO_MARKER) {
    L.Sync.contentToMarker(contentSelector, marker, L.Sync.SYNCED_MARKER_HOVER);
  }

  if (feature.flags & L.Sync.SYNC_MARKER_TO_CONTENT) {
    L.Sync.markerToContent(contentSelector, marker, feature);
  }
});