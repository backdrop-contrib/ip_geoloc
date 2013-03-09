(function ($) {

  // Override Drupal.leaflet.create_point only if a marker tag is supplied.
  Drupal.leaflet._create_point_orig = Drupal.leaflet.create_point;

  Drupal.leaflet.create_point = function(marker) {

    if (!marker.tag) {
      return Drupal.leaflet._create_point_orig(marker);
    }
    var latLng = new L.LatLng(marker.lat, marker.lon);
    this.bounds.push(latLng);
    var lMarker;

    if (marker.icon) {
      var icon = new L.Icon.Tagged(marker.tag, marker.cssClass, {iconUrl: marker.icon.iconUrl});
      // override applicable marker defaults
      if (marker.icon.iconSize) {
        icon.options.iconSize = new L.Point(parseInt(marker.icon.iconSize.x), parseInt(marker.icon.iconSize.y));
      }
      if (marker.icon.iconAnchor) {
        icon.options.iconAnchor = new L.Point(parseFloat(marker.icon.iconAnchor.x), parseFloat(marker.icon.iconAnchor.y));
      }
      if (marker.icon.popupAnchor) {
        icon.options.popupAnchor = new L.Point(parseFloat(marker.icon.popupAnchor.x), parseFloat(marker.icon.popupAnchor.y));
      }
      if (marker.icon.shadowUrl !== undefined) {
        icon.options.shadowUrl = marker.icon.shadowUrl;
      }
      if (marker.icon.shadowSize) {
        icon.options.shadowSize = new L.Point(parseInt(marker.icon.shadowSize.x), parseInt(marker.icon.shadowSize.y));
      }
      if (marker.icon.shadowAnchor) {
        icon.options.shadowAnchor = new L.Point(parseInt(marker.icon.shadowAnchor.x), parseInt(marker.icon.shadowAnchor.y));
      }
      lMarker = new L.Marker(latLng, {icon:icon});
    }
    else {
      lMarker = new L.Marker(latLng);
    }
    return lMarker;
  };

})(jQuery);

L.Icon.Tagged = L.Icon.extend({

  initialize: function (tag, cssClass, options) {
    L.Icon.prototype.initialize.apply(this, [options]);
    this._tag = tag;
    this._cssClass = cssClass;
  },
 
  // Create an icon as per normal, but wrap it in an outerdiv together with the tag.
  createIcon: function() {
    var img = this._createIcon('icon');
    var tag = document.createElement('div');
    tag.innerHTML = this._tag;
    if (this._cssClass) {
      tag.setAttribute('class', this._cssClass);
    }
    var outer = document.createElement('div');
    outer.setAttribute('class', 'leaflet-tagged-marker');
    // The order of these makes little difference
    outer.appendChild(img);
    outer.appendChild(tag);
    return outer;
  },

  createShadow: function() { 
    return this._createIcon('shadow');
  }
});
