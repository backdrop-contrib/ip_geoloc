
(function ($) {

  Drupal.behaviors.addCurrentLocation = {
    attach: function (context, settings) {
      /* Use the geo.js unified API. This covers W3C Geolocation API, Google Gears
       * and some non-conforming specific devices like Palm and Blackberry.
       */
      if (geo_position_js.init()) {
        geo_position_js.getCurrentPosition(getLocation, displayLocationError, {enableHighAccuracy: true});
      }
      else {
        var ip_geoloc_address = new Object;
        ip_geoloc_address['error'] = Drupal.t('IP Geolocation: cannot accurately determine your location. Browser not supported:') + ' '  + navigator.userAgent;
        // Pass error back to PHP 
        $.ajax({
          url: Drupal.settings.basePath + settings.ip_geoloc_menu_callback,
          type: 'POST',
          dataType: 'json',
          data: ip_geoloc_address
        });
      }

      function getLocation(position) {
        var location = new google.maps.LatLng(position.coords.latitude, position.coords.longitude);
        new google.maps.Geocoder().geocode({'latLng': location }, function(response, status) {
          var ip_geoloc_address = new Object;
          ip_geoloc_address['latitude']  = position.coords.latitude;
          ip_geoloc_address['longitude'] = position.coords.longitude;
          ip_geoloc_address['accuracy']  = position.coords.accuracy;

          if (status == google.maps.GeocoderStatus.OK) {
            var google_address = response[0];
            ip_geoloc_address['formatted_address'] = google_address.formatted_address;
            for (var i = 0; i < google_address.address_components.length; i++) {
              var component = google_address.address_components[i];
              if (component.long_name != null) {
                var type = component.types[0];
                ip_geoloc_address[type] = component.long_name;
                if (type == 'country' && component.short_name != null) {
                  ip_geoloc_address['short_name'] = component.short_name;
                }
              }
            }
            //alert('Received address: ' + ip_geoloc_address['formatted_address']);
          }
          else {
            ip_geoloc_address['error'] = Drupal.t('IP Geolocation: Google address lookup failed with status code') + ' ' + status;
          }
          // Pass lat/long, accuracy and address back to Drupal
          $.ajax({
            url: Drupal.settings.basePath + settings.ip_geoloc_menu_callback,
            type: 'POST',
            dataType: 'json',
            data: ip_geoloc_address
          });
        });
      }

      function displayLocationError(error) {
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
        alert("IP Geolocation: getCurrentPosition() returned error code " + error.code + ": " + text);
      }

    }
  }
})(jQuery);
