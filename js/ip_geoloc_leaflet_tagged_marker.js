/* This thing is getting a bit big and complicated... time to refactor! */

(function ($) {

  Backdrop.leaflet._create_point_orig = Backdrop.leaflet.create_point;

  Backdrop.leaflet.create_point = function(marker, lMap) {

    // Follow create_point() in leaflet.backdrop.js
    var latLng = new L.LatLng(marker.lat, marker.lon);
    latLng = latLng.wrap();
    lMap.bounds.push(latLng);

    var options = { title: marker.tooltip, riseOnHover: true };
    if (marker.zIndex) {
      options.zIndexOffset = marker.zIndex;
    }
    if (marker.regions) {
      options.regions = marker.regions;
    }
    if (marker.aggregationValue) {
      options.aggregationValue = marker.aggregationValue;
    }

    if (!marker.tag) {
      // Handle cases where no tag is required and icon is default or none.
      if (marker.icon === false) {
        // No marker. Need to create an icon "stub" or we'll have no map at all!
        options.icon = new L.Icon({iconUrl: '//'});
        return new L.Marker(latLng, options);
      }
      if (!marker.icon) {
        // Marker with default img, without tag.
        // Note: marker.specialChar cannot be handled in this case and is ignored.
        return new L.Marker(latLng, options);
      }
    }
    if (marker.icon === false) {
      // Marker without img, but with tag. marker.specialChar does not apply.
      var divIcon = new L.DivIcon({html: marker.tag, className: marker.cssClass});
      // Prevent div style tag being set, so that upper left corner becomes anchor.
      divIcon.options.iconSize = null;
      options.icon = divIcon;
      return new L.Marker(latLng, options);
    }

    if (marker.tag && !marker.icon) {
      // Use default img, custom tag the marker.
      options.icon = new L.Icon.Tagged(marker.tag, marker.specialChar, {className: marker.cssClass, specialCharClass: marker.special_char_class});
      return new L.Marker(latLng, options);
    }
    // Custom img and custom tag or specialChar.
    var icon = marker.tag || marker.specialChar || marker.specialCharClass
      ? new L.Icon.Tagged(marker.tag, marker.specialChar, {iconUrl: marker.icon.iconUrl, className: marker.cssClass, specialCharClass: marker.specialCharClass})
      : new L.Icon({iconUrl: marker.icon.iconUrl});

    // All of this is like create_point() in leaflet.backdrop.js, but with tooltip.
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
    options.icon = icon;
    return new L.Marker(latLng, options);
  };

})(jQuery);

L.Icon.Tagged = L.Icon.extend({

  initialize: function (tag, specialChar, options) {
    L.Icon.prototype.initialize.apply(this, [options]);
    this._tag = tag;
    this._specialChar = specialChar;
  },

  // Create an icon as per normal, but wrap it in an outerdiv together with the tag.
  createIcon: function() {
    if (!this.options.iconUrl) {
      var iconDefault = new L.Icon.Default();
      this.options.iconUrl = iconDefault._getIconUrl('icon');
      this.options.iconSize = iconDefault.options.iconSize;
      this.options.iconAnchor = iconDefault.options.iconAnchor;
      // Does this work?
      this.options.popupAnchor = iconDefault.options.popupAnchor;
      this.options.shadowSize = iconDefault.options.shadowSize;
    }

    var outer = document.createElement('div');
    outer.setAttribute('class', 'leaflet-tagged-marker');
    // The order of appending img, div and i makes little difference.

    var img = this._createIcon('icon');
    outer.appendChild(img);

    if (this._specialChar || this.options.specialCharClass) {
      // Convention seems to be to use the i element.
      // Other elements like div and span work also, just make sure that
      // display:block is set implicitly or explicitly.
      var specialChar = document.createElement('i');
      specialChar.innerHTML = this._specialChar ? this._specialChar.trim() : '';
      if (this.options.specialCharClass) {
        specialChar.setAttribute('class', this.options.specialCharClass);
      }
      outer.appendChild(specialChar);
    }
    if (this._tag) {
      var tag = document.createElement('div');
      tag.innerHTML = this._tag;
      if (this.options.className) {
        tag.setAttribute('class', this.options.className);
      }
      outer.appendChild(tag);
    }

    return outer;
  },

  createShadow: function() {
    return this._createIcon('shadow');
  }
});
