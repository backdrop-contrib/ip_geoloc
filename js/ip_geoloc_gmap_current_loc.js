
(function ($) {

  Drupal.behaviors.addGMapCurrentLocation = {
    attach: function (context, settings) {

      // Start with a map canvas, then add marker and balloon with address info
      // when the geo-position comes in.
      var mapOptions = settings.ip_geoloc_current_location_map_options;
      if (!mapOptions) {
        mapOptions = { mapTypeId: google.maps.MapTypeId.ROADMAP, zoom: 15 };
      }
      var map = new google.maps.Map(document.getElementById(settings.ip_geoloc_current_location_map_div), mapOptions);
    
      /* Use the geo.js unified API. This covers W3C Geolocation API, Google Gears
       * and some non-conforming specific devices like Palm and Blackberry.
       */
      if (geo_position_js.init()) {
        geo_position_js.getCurrentPosition(displayMap, displayMapError, {enableHighAccuracy: true});
      }
      else {
        // Don't pop up annoying alert. Just show blank map of the world.
        map.setZoom(0);
        map.setCenter(new google.maps.LatLng(0, 0));
      }

      function displayMap(position) {
        var coords = position.coords;
        var center = new google.maps.LatLng(coords.latitude, coords.longitude);
        map.setCenter(center);
        var marker = new google.maps.Marker({ map: map, position: center });
        new google.maps.Geocoder().geocode({'latLng': center}, function(response, status) {
          if (status == google.maps.GeocoderStatus.OK) {
            addressText = response[0]['formatted_address'];
          }
          else {
            addressText = Drupal.t('IP Geolocation displayMap(): Google address lookup failed with status code ') + status;
          }
          // (lat, long) and address are revealed when clicking marker
          var latLongText = Drupal.t('lat.') + " " + coords.latitude + ", " + Drupal.t('long.') + " " + coords.longitude + "<br/>" + Drupal.t('accuracy') + " " + coords.accuracy + " m";
          var infoPopUp = new google.maps.InfoWindow({ content: addressText + "<br/>" + latLongText });
          google.maps.event.addListener(marker, "click", function() { infoPopUp.open(map, marker) });
        });
      }

      function displayMapError(error) {
        switch (error.code) {
          case 1:
            text = Drupal.t("user denied permission to share location");
            break;
          case 2:
            text = Drupal.t("position unavailable (connection lost?)");
            break;
          case 3:
            text = Drupal.t("timeout");
            break;
          default:
            text = Drupal.t("unknown error");
        }
        alert("IP Geolocation, current location map: getCurrentPosition() returned error code " + error.code + ": " + text);
      }
    }
  }
})(jQuery);

