
jQuery(document).bind('leaflet.feature', function(event, marker, feature) {
  // marker is the lFeature just added to the map, it could be a polygon too.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // The same code is used for cross-highlighting. See ip_geoloc_leaflet_sync_content.js
  if (feature.feature_id) {
    marker.feature_id = feature.feature_id;
  }
});

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  // Handle click event.
  if (map.settings.gotoContentOnClick) {
    for (var leaflet_id in lMap._layers) {
      lMap._layers[leaflet_id].on('click', function(e) {
        var id = e.target ? e.target.feature_id : null;
        if (!id && e.layer) id = e.layer.feature_id;
        if (id) {
          document.location.href = Drupal.settings.basePath + 'node/' + id;
        }
      });
    }
  }
  // Handle mouseover and mouseout events
  if (map.settings.openBalloonsOnHover || map.settings.polygon_fill_opacity_on_hover) {
    for (var leaflet_id in lMap._layers) {
      var layer = lMap._layers[leaflet_id];
      if (layer._path) {
        // Store the current fill opacity, so we can revert to it on mouseout
        layer._fillOpacity = layer._path.getAttribute('fill-opacity');
        layer._strokeWidth = layer._path.getAttribute('stroke-width');
      }
      layer.on('mouseover', function(e) {
        if (map.settings.openBalloonsOnHover) {
          this.openPopup();
        }
        // setStyle is only available on some features types.
        if (map.settings.polygon_fill_opacity_on_hover && typeof(this.setStyle) == 'function') {
          this.setStyle({
            fillOpacity: map.settings.polygon_fill_opacity_on_hover,
            weight: map.settings.polygon_line_weight_on_hover,
         });
        }
      });
    }
  }
  if (map.settings.polygon_fill_opacity_on_hover) {
    for (var leaflet_id in lMap._layers) {
      var layer = lMap._layers[leaflet_id];
      // setStyle is only available on some features types.
      if (typeof(layer.setStyle) == 'function') {
        layer.on('mouseout', function(e) {
          this.setStyle({
            fillOpacity: this._fillOpacity,
            weight: this._strokeWidth,
          });
        })
      }
    }
  }

});