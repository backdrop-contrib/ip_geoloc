
jQuery(document).bind('leaflet.feature', function(event, marker, feature) {
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // see also ip_geoloc_leaflet_sync_content.js
  if (feature.feature_id) {
    marker.feature_id = feature.feature_id;
  }

  marker.on('click', function(event) {
    document.location.href = 'node/' + this.feature_id;
  });
});
