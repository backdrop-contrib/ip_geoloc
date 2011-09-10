
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
        if (++i == 1) { // use to first, i.e. most recent, visitor to center the map
          map.setCenter(position);
          mouseOverText = Drupal.t('Latest visitor - you?');
        }
        else {
          mouseOverText = Drupal.t('Visitor #@i', { '@i': i });
        }
        marker = new google.maps.Marker({ map: map, position: position, title: mouseOverText });
        balloonTexts['LL' + position] = 
          Drupal.t('IP address: @ip', { '@ip':  ip }) + '<br/>' +
          locations[ip].formatted_address + '<br/>' +
          Drupal.t('#visits: @count, last visit: @date', { '@count': locations[ip].visit_count, '@date': locations[ip].last_visit });
        //var content = "Lat/long: " + event.latLng + "<br/>" + balloonTexts['LL' + event.latLng]
        google.maps.event.addListener(marker, 'click',  function(event) {
          var lat  = event.latLng.lat().toFixed(4);
          var long = event.latLng.lng().toFixed(4);
          new google.maps.InfoWindow({
            content: Drupal.t('Lat/long: @lat/@long', { '@lat': lat, '@long': long }) + '<br/>' + balloonTexts['LL' + event.latLng], 
            position: event.latLng 
          }).open(map);
        });
      }

    }
  }
})(jQuery);

