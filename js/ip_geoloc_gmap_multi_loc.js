
(function ($) {

  Drupal.behaviors.addGMapMultiLocation = {
    attach: function (context, settings) {

      var mapOptions = settings.ip_geoloc_multi_location_map_options;
      if (!mapOptions) {
        mapOptions = { mapTypeId: google.maps.MapTypeId.ROADMAP, zoom: 2 };
      }
      var map = new google.maps.Map(document.getElementById(settings.ip_geoloc_multi_location_map_div), mapOptions);

      var locations = settings.ip_geoloc_locations;
      var balloonTexts = [];
      var i = 0;
      for (ip in locations) {
        var position = new google.maps.LatLng(locations[ip].latitude, locations[ip].longitude);
        if (i++ == 0) { // use to first, i.e. most recent, visitor to center the map
          map.setCenter(position);
          marker = new google.maps.Marker({ map: map, position: position, title: Drupal.t('Visitor #') + i + ': ' + Drupal.t('you?') });
        } else {
          marker = new google.maps.Marker({ map: map, position: position, title: Drupal.t('Visitor #') + i });
        }
        balloonTexts['LL' + position] = Drupal.t('IP address: ') + ip + '<br/>' + locations[ip].formatted_address + '<br/>'
          + Drupal.t('#visits: ') + locations[ip].visit_count + ', ' + Drupal.t('last visit: ') + locations[ip].last_visit;
        google.maps.event.addListener(marker, 'click',  function(event) {
          new google.maps.InfoWindow({ content: "Lat/long: " + event.latLng + "<br/>" + balloonTexts['LL' + event.latLng], position: event.latLng }).open(map);
        });
      }

    }
  }
})(jQuery);

