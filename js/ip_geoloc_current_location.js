
(function ($) {

  Backdrop.behaviors.addCurrentLocation = {
    attach: function (context, settings) {
      ip_geoloc_getCurrentPosition(
        settings.ip_geoloc_menu_callback,
        settings.ip_geoloc_reverse_geocode,
        settings.ip_geoloc_refresh_page
      );
    }
  }

})(jQuery);

function ip_geoloc_getCurrentPosition(callbackUrl, reverseGeocode, refreshPage) {

  if (typeof(getCurrentPositionCalled) !== 'undefined') {
    // Been here, done that (can happen in AJAX context).
    // Stop throbber
    jQuery('#set-location-form .ajax-progress.ajax-progress-throbber').hide();
    return;
  }

  if (navigator.geolocation) {
    var startTime = (new Date()).getTime();
    navigator.geolocation.getCurrentPosition(getLocation, handleLocationError, {enableHighAccuracy: true, timeout: 19000});
    getCurrentPositionCalled = true;
  }
  else {
    var data = new Object;
    data['error'] = Backdrop.t('IPGV&M: device does not support W3C API.');
    callbackServer(callbackUrl, data, false);
  }

  function getLocation(position) {

    if (window.console && window.console.log) { // Does not work on IE8
      var elapsedTime = (new Date()).getTime() - startTime;
      window.console.log(Backdrop.t('!time s to determine visitor coords', { '!time' : elapsedTime/1000 }));
    }
    var ip_geoloc_address = new Object;
    ip_geoloc_address['latitude']  = position.coords.latitude;
    ip_geoloc_address['longitude'] = position.coords.longitude;
    ip_geoloc_address['accuracy']  = position.coords.accuracy;

    if (!reverseGeocode) {
      // Pass lat/long back to Backdrop without street address.
      callbackServer(callbackUrl, ip_geoloc_address, refreshPage);
      return;
    }
    else if (typeof(google) === 'undefined' || typeof(google.maps) === 'undefined') {
      ip_geoloc_address['error'] = Backdrop.t('getLocation(): Google Maps API not loaded, cannot reverse-geocode position to address')
      callbackServer(callbackUrl, ip_geoloc_address, true);
      return;
    }
    // Reverse-geocoding of lat/lon requested.
    startTime = (new Date()).getTime();
    var location = new google.maps.LatLng(position.coords.latitude, position.coords.longitude);
    new google.maps.Geocoder().geocode({'latLng': location }, function(response, status) {

      if (status === google.maps.GeocoderStatus.OK) {
        var google_address = response[0];
        ip_geoloc_address['formatted_address'] = google_address.formatted_address;
        for (var i = 0; i < google_address.address_components.length; i++) {
          var component = google_address.address_components[i];
          if (component.long_name !== null) {
            var type = component.types[0];
            ip_geoloc_address[type] = component.long_name;
            if (type === 'country' && component.short_name !== null) {
              ip_geoloc_address['country_code'] = component.short_name;
            }
          }
        }
      }
      else {
        var error = ''; // from response or status?
        if (window.console && window.console.log) {
          window.console.log(Backdrop.t('IPGV&M: Google Geocoder returned error !code.', { '!code': status }));
        }
        ip_geoloc_address['error'] = Backdrop.t('getLocation(): Google Geocoder address lookup failed with status code !code. @error', { '!code': status, '@error': error });
        refreshPage = false;
      }
      if (window.console && window.console.log && ip_geoloc_address['formatted_address']) {
        var elapsedTime = (new Date()).getTime() - startTime;
        window.console.log(Backdrop.t('!time s to reverse-geocode to !address', { '!time' : elapsedTime/1000, '!address' : ip_geoloc_address['formatted_address'] }));
      }

      // Pass lat/long, accuracy and address back to Backdrop
      callbackServer(callbackUrl, ip_geoloc_address, refreshPage);
    });
  }

  function handleLocationError(error) {
    var data = new Object;
    data['error'] = Backdrop.t('getCurrentPosition() returned error !code: !text. Browser: @browser',
      {'!code': error.code, '!text': error.message, '@browser': navigator.userAgent});
    // Pass error back to PHP rather than alert();
    callbackServer(callbackUrl, data, false);
  }

  function callbackServer(callbackUrl, data, refresh_page) {
    // Stop throbber
    jQuery('#set-location-form .ajax-progress.ajax-progress-throbber').hide();

    // For drupal.org/project/js module, if enabled.
    data['js_module'] = 'ip_geoloc';
    data['js_callback'] = 'current_location';

    jQuery.ajax({
      url: callbackUrl,
      type: 'POST',
      dataType: 'json',
      data: data,
      success: function (serverData, textStatus, http) {
        if (window.console && window.console.log) {
          if (serverData && serverData.messages && serverData.messages['status']) {
            // When JS module is used, it collects msgs via backdrop_get_messages().
            var messages = serverData.messages['status'].toString();
            // Remove any HTML markup.
            var msg = Backdrop.t('From server, via JS: ') + jQuery('<p>' + messages + '</p>').text();
          }
          else {
            //var msg = Backdrop.t('Server confirmed with: @status', { '@status': textStatus });
          }
          if (msg) window.console.log(msg);
        }
        if (refresh_page) {
          window.location.reload();
        }
      },
      error: function (http, textStatus, error) {
        // 404 may happen intermittently and when Clean URLs isn't enabled
        // 503 may happen intermittently, see [#2158847]
        var msg = Backdrop.t('IPGV&M, ip_geoloc_current_location.js @status: @error (@code)', { '@status': textStatus, '@error': error, '@code': http.status });
        if (window.console && window.console.log) {
          window.console.log(msg);
        }
        if (http.status > 0 && http.status !== 200 && http.status !== 404 && http.status !== 503) {
          alert(msg);
        }
      },
      complete: function(http, textStatus) {
        var msg = !http.responseText ? '' : http.responseText.replace(/(^"|"$)/g, '');
        if (msg !== '') {
          window.console.log(Backdrop.t('AJAX call completed with @status and response: @msg', { '@status': textStatus, '@msg': msg }));
        }
        else {
          window.console.log(Backdrop.t('AJAX call completed with @status and empty response.', { '@status': textStatus }));
        }
      }
    });
  }
}
