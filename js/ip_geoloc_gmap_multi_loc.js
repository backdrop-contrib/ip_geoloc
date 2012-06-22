
(function ($) {

  Drupal.behaviors.addGMapMultiLocation = {
    attach: function (context, settings) {

      var mapOptions = settings.ip_geoloc_multi_location_map_options;
      if (!mapOptions) {
        mapOptions = { mapTypeId: google.maps.MapTypeId.ROADMAP, zoom: 2 };
      }
      map = new google.maps.Map(document.getElementById(settings.ip_geoloc_multi_location_map_div), mapOptions);

      centerSet = false;

      if (settings.ip_geoloc_multi_location_show_visitor_location && geo_position_js.init()) {
        // Center the map on the user's current location, using the geo.js unified API.
        geo_position_js.getCurrentPosition(setMapCenterAndMarker, handlePositionError, {enableHighAccuracy: true});
      }
      var locations = settings.ip_geoloc_locations;
      if (!centerSet && locations.length == 0) {
        // Don't pop up annoying alert. Just show blank map of the world.
        map.setZoom(0);
        map.setCenter(new google.maps.LatLng(0, 0));
      }

      var i = 1;
      for (var key in locations) {
        var mouseOverText = Drupal.t('Location #@i', { '@i': i++ });
        var position = new google.maps.LatLng(locations[key].latitude, locations[key].longitude);
        if (!centerSet) {
          // If the visitor's location could not be determined, center map on
          // the first location, if there are any at all.
          map.setCenter(position);
          centerSet = true;
        }

        marker = new google.maps.Marker({ map: map, position: position, title: mouseOverText });

        google.maps.event.addListener(marker, 'click',  function(event) {
          new google.maps.InfoWindow({
            content: locations[key].balloonText,
            position: event.latLng
          }).open(map);
        });
      }

      function setMapCenterAndMarker(visitor_position) {
        var center = new google.maps.LatLng(visitor_position.coords.latitude, visitor_position.coords.longitude);
        var balloonText = Drupal.t('Your current position');
        map.setCenter(center);
        centerSet = true;

        var pinColor = "00EE00";
        var pinImage = new google.maps.MarkerImage("http://chart.apis.google.com/chart?chst=d_map_pin_letter&chld=%E2%80%A2|" + pinColor,
          new google.maps.Size(21, 34), new google.maps.Point(0,0), new google.maps.Point(10, 34));

        centerMarker = new google.maps.Marker({ icon: pinImage, map: map, position: center, title: balloonText });
        google.maps.event.addListener(centerMarker, 'click',  function(event) {
          new google.maps.InfoWindow({
            content: balloonText,
            position: event.latLng
          }).open(map);
        });
      }

      function handlePositionError(error) {
        //alert(Drupal.t('IP Geolocation, multi-location map: getCurrentPosition() returned error !code', {'!code': error.code}));
      }
    }
  }
})(jQuery);
