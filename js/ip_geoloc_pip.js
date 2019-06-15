
var visitorMarker = null;

function point_in_polygon(latlng, polygon) {
  const noVertices = polygon.length
  let c = false
  for (let i = 0, j = noVertices - 1; i < noVertices; j = i++) {
    if (((polygon[i].lat > latlng.lat) != (polygon[j].lat > latlng.lat))
        && (latlng.lng < (polygon[j].lng - polygon[i].lng) * (latlng.lat- polygon[i].lat) / (polygon[j].lat - polygon[i].lat) + polygon[i].lng)) {
      c = !c
    }
  }
  return c;
}

jQuery(document).bind('leaflet.feature', function(event, lFeature, feature) {
  if (feature.title) {
    lFeature.title = feature.title;
  }
  if (feature.type == 'point' && feature.isVisitor) {
    visitorMarker = lFeature
  }
})

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

    if (!visitorMarker || !visitorMarker._popup) {
      return
    }
    let layer
    for (let leaflet_id in lMap._layers) {
      layer = lMap._layers[leaflet_id];
      if (layer._latlngs) {
        for (let i = 0; i < layer._latlngs.length; i++) {
          // Apply PiP on visitor being inside polygons associated with layer.
          if (point_in_polygon(visitorMarker._latlng, layer._latlngs[i])) {
            visitorMarker._popup._content = visitorMarker._popup._content.replace('[visitor-location:surrounding-polygon]', layer.title)
            visitorMarker.openPopup()
            return
          }
        }
      }
    }
    visitorMarker._popup._content = "You are not within any of the<br/>regions shown on this map.";
    visitorMarker.openPopup()
})