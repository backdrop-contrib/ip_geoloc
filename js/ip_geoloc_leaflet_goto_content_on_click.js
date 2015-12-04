
jQuery(document).bind('leaflet.feature', function(event, marker, feature) {
  
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // see also ip_geoloc_leaflet_sync_content.js
  if (feature.feature_id) {
    marker.feature_id = feature.feature_id;
  }

  marker.on('click', function(e) {
    var leafletSettings = Drupal.settings.leaflet;
    for (var i = 0; i < leafletSettings.length; i++) {
      if ((this._map === leafletSettings[i].lMap) && leafletSettings[i].map.settings.gotoContentOnClick) {
        document.location.href = Drupal.settings.basePath + 'node/' + this.feature_id;
        return;
      }
    }
  });

  marker.on('mouseover', function(e) {
    var leafletSettings = Drupal.settings.leaflet;
    for (var i = 0; i < leafletSettings.length; i++) {
      if ((this._map === leafletSettings[i].lMap) && leafletSettings[i].map.settings.openBalloonsOnHover) {
        marker.openPopup();
        return;
      }
    }
  });

});

