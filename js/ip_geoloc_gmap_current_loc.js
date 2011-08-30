
(function ($) {

  Drupal.behaviors.addGMapCurrentLocation = {
    attach: function (context, settings) {
      /* Use the geo.js unified API. This covers W3C Geolocation API, Google Gears
       * and some non-conforming specific devices like Palm and Blackberry.
       */
      if (geo_position_js.init()) {
        geo_position_js.getCurrentPosition(displayMap, displayError, {enableHighAccuracy: true});
      }
      else {
        alert(Drupal.t("Error: geo_position_js is not available."));
      }

      var mapOptions = {
        mapTypeId: google.maps.MapTypeId.ROADMAP,
        disableDefaultUI: true,
        zoom: 15,
        zoomControl: true
      };
      map = new google.maps.Map(document.getElementById(settings.ip_geoloc_div_map), mapOptions);

      function displayMap(position) {
        crds = position.coords;
        var center = new google.maps.LatLng(crds.latitude, crds.longitude);
        map.setCenter(center);
        var marker = new google.maps.Marker({ map: map, position: center });
        new google.maps.Geocoder().geocode({'latLng': center}, function(response, status) {
          if (status == google.maps.GeocoderStatus.OK) {
            addressText = response[0]['formatted_address'];
          }
          else {
            addressText = Drupal.t('IP Geolocation: Google address lookup failed with status code ') + status;
          }
          // (lat, long) and address are revealed when clicking marker
          var latLongText = Drupal.t('lat.') + " " + crds.latitude + ", " + Drupal.t('long.') + " " + crds.longitude + "<br/>" + Drupal.t('accuracy') + " " + crds.accuracy + " m";
          var infoPopUp = new google.maps.InfoWindow({ content: addressText + "<br/>" + latLongText });
          google.maps.event.addListener(marker, "click", function() { infoPopUp.open(map, marker) });
        });
      }

      function displayError(error) {
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

