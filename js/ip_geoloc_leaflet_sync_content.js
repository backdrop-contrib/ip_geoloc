// Take advantage of Leaflet's Class idiom
L.Sync =  L.Class.extend({

	statics: {
    SYNC_CONTENT_TO_MARKER: 1 << 1,
    SYNC_MARKER_TO_CONTENT: 1 << 2,
    SYNC_MARKER_TO_CONTENT_WITH_POPUP: 1 << 3,

    SYNCED_CONTENT_HOVER: 'synced-content-hover',
    SYNCED_MARKER_HOVER : 'synced-marker-hover',
    SYNCED_MARKER_HIDDEN: 'leaflet-marker-hidden',
  },

  options: {
    // Maybe one day
	},

	initialize: function (map, options) {
    L.setOptions(this, options);
    this.map = map;
    this.lastMarker = null;
    this.zoom = map.getZoom();
	},

  closePopup: function(marker) {
    if (marker && marker.closePopup) {
      marker.closePopup();
    }
  },

  hide: function(marker) {
    // Not using setOpacity() as it does not work for non-marker geometries.
    this.addClass(marker, L.Sync.SYNCED_MARKER_HIDDEN);
    this.closePopup(marker);
    if (marker === this.lastMarker) {
      this.lastMarker = null;
    }
  },

  hideIfWasInCluster: function(marker) {
    if (marker && marker.wasInCluster) {
      this.hide(marker);
    }
  },

  show: function(marker) {
    this.removeClass(marker, L.Sync.SYNCED_MARKER_HIDDEN);
  },

  highlight: function(marker) {
    this.addClass(marker, L.Sync.SYNCED_CONTENT_HOVER);
  },

  unhighlight: function(marker) {
    this.removeClass(marker, L.Sync.SYNCED_CONTENT_HOVER);
  },

  syncContentToMarker: function(contentSelector, marker) {
    marker.on('mouseover', function(event) {
      jQuery(contentSelector).addClass(L.Sync.SYNCED_MARKER_HOVER);
    });
    marker.on('mouseout', function(event) {
      jQuery(contentSelector).removeClass(L.Sync.SYNCED_MARKER_HOVER);
    });
  },

  syncMarkerToContent: function(contentSelector, marker) {
    var sync = this;

    marker.on('popupclose', function(event) {
      var marker = event.target;
      sync.unhighlight(marker)
      if (marker._icon && (marker.flags & L.Sync.SYNC_MARKER_TO_CONTENT_WITH_POPUP)) {
        marker._popup.options.offset.y = sync.popupOffsetY;
      }
      sync.hideIfWasInCluster(marker);
    });

    // Using bind() as D7 core's jQuery is old and does not support on()
    jQuery(contentSelector).bind('mouseover', function(event) {
      sync.handleContentMouseOver(marker);
    });
  },

  handleContentMouseOver: function(marker) {
    if (marker === null || marker === this.lastMarker) {
      return;
    }

    if (this.lastMarker) {
      this.unhighlight(this.lastMarker);
      this.hideIfWasInCluster(this.lastMarker);
    }

    if (this.hasMarkerClusters() && !marker.wasInCluster) {
      marker.wasInCluster = !marker._map;
    }
    if (!marker._map) {
      marker.addTo(this.map);
    }

    var point = marker.getLatLngs ? marker.getLatLngs()[0] : marker.getLatLng();
    if (!this.map.getBounds().contains(point)) {
      this.map.panTo(point);
    }
    // Make geometry visible, in case it was invisible.
    this.show(marker)
    this.highlight(marker);

    if (marker.flags & L.Sync.SYNC_MARKER_TO_CONTENT_WITH_POPUP) {
      if (marker._icon) {
        // Adjust popup position for markers, not other geometries.
        this.popupOffsetY = marker._popup.options.offset.y;
        marker._popup.options.offset.y -= 20;
      }
      marker.openPopup();
    }
    if (marker._icon && marker._icon.style) {
      // This does NOT work in most browsers.
      marker._bringToFront();
    }
    this.lastMarker = marker;
  },

  addClass: function(marker, className) {
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

  hasMarkerClusters: function() {
    for (var id in this.map._layers) {
      if (this.map._layers[id]._topClusterLevel) {
        return true;
      }
    }
    return false;
  },

  getMarkersInClusters: function() {
    for (var id in this.map._layers) {
      if (this.map._layers[id]._topClusterLevel) {
        return this.map._layers[id]._topClusterLevel.getAllChildMarkers();
      }
    }
    return [];
  },

  getMarkerElements: function(marker) {
    var elements = [];
    if (marker) {
      if (marker._icon) {
        elements.push(marker._icon);
      }
      if (marker._container) {
        elements.push(marker._container);
      }
      if (marker._layers) {
        for (var key in marker._layers) elements.push(marker._layers[key]._container);
      }
    }
    return elements;
  }
});

// Gets triggered before 'leaflet.map'. Extend marker data for further use.
jQuery(document).bind('leaflet.feature', function(event, marker, feature) {
  // marker is the feature just added to the map, it could be a polygon too.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  if (feature.feature_id) {
    marker.feature_id = feature.feature_id;
  }
  if (feature.flags) {
    marker.flags = feature.flags;
  }
});

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  var sync = new L.Sync(lMap, {});

  lMap.on('zoomend', function(event) {

    if (event.target.getZoom) {
      var isZoomOut = event.target.getZoom() < sync.zoom;
      sync.zoom = event.target.getZoom();
    }
    // Hide highlighted marker on zoom when:
    // 1) zooming out (as marker may be abosrbed by cluster) or
    // 2) zooming-in when highlighted marker was in cluser
    if (sync.lastMarker && (isZoomOut || sync.lastMarker.wasInCluster)) {
      sync.unhighlight(sync.lastMarker);
      sync.hide(sync.lastMarker);
    }

    var clusterMarkers = sync.getMarkersInClusters();
    for (var i = 0; i < clusterMarkers.length; i++) {
      clusterMarkers[i].wasInCluster = false;
    }
  });

  if (map.settings.revertLastMarkerOnMapOut) {
    lMap.on('mouseout', function(event) {
      if (sync.lastMarker) {
        sync.unhighlight(sync.lastMarker);
        sync.hideIfWasInCluster(sync.lastMarker);
        //event.target.closePopup();
      }
    });
  }

  var clusterMarkers = sync.getMarkersInClusters();
  // Convert lMap._layers to array.
  var otherMarkers = Object.keys(lMap._layers).map(function(key) { return lMap._layers[key] });
  var allMarkers = clusterMarkers.concat(otherMarkers);

  for (var i = 0; i < allMarkers.length; i++) {
    var marker = allMarkers[i];
    if (marker.flags) {
      // A CSS class, not an ID as multiple markers may be attached to same node.
      var contentSelector = ".sync-id-" + marker.feature_id;
 
      if (marker.flags & L.Sync.SYNC_CONTENT_TO_MARKER) {
        sync.syncContentToMarker(contentSelector, marker);
      }
      if (marker.flags & L.Sync.SYNC_MARKER_TO_CONTENT) {
        sync.syncMarkerToContent(contentSelector, marker);
      }
    }
  }

});
