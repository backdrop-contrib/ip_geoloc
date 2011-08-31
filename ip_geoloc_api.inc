<?php

/**
 * @file
 * API functions of IP geolocation module
 *
 * Reusable functions that may be called from other modules.
 */

define('IP_GEOLOC_GOOGLE_GEARS_INIT', 'http://code.google.com/apis/gears/gears_init.js');
define('IP_GEOLOC_GEO_JS', 'http://geo-location-javascript.googlecode.com/svn/trunk/js/geo.js');
define('IP_GEOLOC_GOOGLE_MAPS',     'http://maps.google.com/maps/api/js?sensor=false');
define('IP_GEOLOC_GOOGLE_MAPS_GPS', 'http://maps.google.com/maps/api/js?sensor=true');

/**
 * Store the supplied IP geolocation info on the database, replacing existing
 * info only if the new info has more detail or when forced by the system
 * variable ip_geoloc_refresh_at_new_session.
 *
 * @param $location, array with up to 13 location info fields; must at least
 *   contain a non-empty $location['ip_address'] for anything to happen
 * @return
 *   0, when no insert or update was necessary
 *   SAVED_NEW (=1), when a new location record was inserted into the db
 *   SAVED_UPDATED (=2), when an existing location record was updated
 *   FALSE, when a db insert or db update failed
 */
function ip_geoloc_store_location($location) {
  $result = 0;
  if (empty($location['ip_address'])) {
    ip_geoloc_debug('IP Geolocation: location object does not contain IP address, so was not stored.');
  }
  elseif (empty($location['formatted_address'])) {
    ip_geoloc_debug('IP Geolocation: location object does not contain formatted address, so was not stored.');
  }
  else {
    if (!isset($location['latitude']) || !isset($location['latitude'])) {
      drupal_set_message(t('IP Geolocation: lat or long missing for IP address %ip', array('%ip' => $location['ip_address'])));
    }
    // See if this IP is already on the db
    $result = db_query("SELECT * FROM {ip_geoloc} WHERE ip_address = :ip", array(':ip' => $location['ip_address']));
    $db_location = $result->fetchAssoc();

    if ($db_location == FALSE) { // New entry, insert
      ip_geoloc_debug(t('Adding new info to geolocaton db: ') . ip_geoloc_pretty_print($location));
      $result = drupal_write_record('ip_geoloc', $location);
    }
    else {
      $location['id'] = $db_location['id'];
      // When updating, drupal_write_record() does not erase fields not present in $location
      $empty_location['latitude'] = '';
      $empty_location['longitude'] = '';
      $empty_location['country'] = '';
      $empty_location['country_code'] = '';
      $empty_location['region'] = '';
      $empty_location['region_code'] = '';
      $empty_location['city'] = '';
      $empty_location['locality'] = '';
      $empty_location['route'] = '';
      $empty_location['street_number'] = '';
      $empty_location['postal_code'] = '';
      $empty_location['administrative_area_level_1'] = '';
      $empty_location['formatted_address'] = '';
      $full_location = array_merge($empty_location, $location);
      ip_geoloc_debug(t('Updating geolocation db with: ') . ip_geoloc_pretty_print($full_location));
      $result = drupal_write_record('ip_geoloc', $full_location, 'id');
    }
    if ($result === FALSE) {
      drupal_set_message(t('IP Geolocation: could not update geolocation information for IP address %ip', array('%ip' => $location['ip_address'])), 'warning');
    }
  }
  return $result;
}

/**
 * Outputs an HTML div placeholder into which will be injected a map of the
 * area around the supplied lat,long coordinates.
 *
 * @param $latitude,  string or double, e.g. "-37.87" or -37.87
 * @param $longitude, string or double, e.g. "144.98" or 144.98
 * @param $div_id, id of the div placeholder, can be anything as long as it's unique
 * @param $style, CSS style applied to the div, e.g "height:250px; width:300px"
 */
function ip_geoloc_output_map($latitude, $longitude, $div_id = 'ip-geoloc-map', $style = '') {
  drupal_add_js(IP_GEOLOC_GOOGLE_MAPS);

  // Add javascript to produce map centered around ($latitidue, $longitude)
  $js_file = drupal_get_path('module', 'ip_geoloc') . '/js/ip_geoloc_gmap.js';
  drupal_add_js($js_file, array('weight' => 1));

  $script_code = "displayGMap($latitude, $longitude, '$div_id');";
  drupal_add_js($script_code, array('type' => 'inline', 'scope' => 'footer'));

  $map_placeholder = "<div id='$div_id'" . (empty($style) ? '' : " style='$style'") . '></div>';
  return $map_placeholder;
}

/**
 * Outputs an HTML div placeholder into which is injected a map of the area
 * around the visitor's current location.
 * The map contains a marker for the current location, which when clicked
 * reveals latitude and longitude, as well as the street address and the
 * accuracy of the position shown.
 *
 * @param $div_id, id of the div placeholder, can be anything as long as it's unique
 * @param $style, CSS style applied to the div, e.g "height:200px; width:300px"
 */
function ip_geoloc_output_map_current_location($div_id = 'ip-geoloc-map-current-location', $map_style = '') {
  drupal_add_js(IP_GEOLOC_GOOGLE_GEARS_INIT);
  drupal_add_js(IP_GEOLOC_GEO_JS);
//drupal_add_js(drupal_get_path('module', 'ip_geoloc') . '/js/geo.js');
  drupal_add_js(IP_GEOLOC_GOOGLE_MAPS_GPS);
  drupal_add_js(array('ip_geoloc_div_map' => $div_id), 'setting');

  // Add javascript to retrieve user's lat,long and draw a gmap centered around it
  $js_file = drupal_get_path('module', 'ip_geoloc') . '/js/ip_geoloc_gmap_current_loc.js';
  drupal_add_js($js_file, array('weight' => 1));

  $map_placeholder = "<div id='$div_id'" . (empty($map_style) ? '' : " style='$map_style'") . '></div>';
  return $map_placeholder;
}

/**
 * Outputs an HTML div placeholder into which will be injected a multi-location
 * map, its locations supplied as an array of lat,long coordinates.
 *
 * @param $locations, array of location objects each containing lat/long pair
 *   and infoText to appear when the marker is clicked
 * @param $div_id, id of the div placeholder, can be anything as long as it's unique
 * @param $style, CSS style applied to the div, e.g "height:250px; width:300px"
 */
function ip_geoloc_output_map_multi_locations($locations, $div_id = 'ip-geoloc-map-multi-locations', $style = '') {
  drupal_add_js(IP_GEOLOC_GOOGLE_MAPS);
  drupal_add_js(array('ip_geoloc_locations' => $locations), 'setting');
  drupal_add_js(array('ip_geoloc_div_map' => $div_id), 'setting');

  $js_file = drupal_get_path('module', 'ip_geoloc') . '/js/ip_geoloc_gmap_multi_loc.js';
  drupal_add_js($js_file, array('weight' => 1));

  $map_placeholder = "<div id='$div_id'" . (empty($style) ? '' : " style='$style'") . '></div>';
  return $map_placeholder;
}

/**
 * Places in $_POST the latitude, longitude and accuracy of the user's current
 * location, as well as the formatted address (a single string) reverse-geocoded
 * from the lat/long.
 *
 * Note: this is done via an asynchronous javascript call, so the result may not
 * yet be available on return from this function, hence the $menu_callback
 * argument. Upon page load the included javascript will, when ready, instigate
 * an AJAX call to the $menu_callback, which should call the function that
 * retrieves the return lat/long and address values from the $_POST variable.
 *
 * See ip_geoloc_current_location_ajax_recipient() for a simple example.
 */
function ip_geoloc_get_current_location($menu_callback = 'ip-geoloc-current-location') {
  drupal_add_js(IP_GEOLOC_GOOGLE_GEARS_INIT);
  drupal_add_js(IP_GEOLOC_GEO_JS);
//drupal_add_js(drupal_get_path('module', 'ip_geoloc') . '/js/geo.js');
  drupal_add_js(IP_GEOLOC_GOOGLE_MAPS_GPS);
  drupal_add_js(array('ip_geoloc_menu_callback' => $menu_callback), 'setting');
  // Add javascript to return user's (lat,long), accuracy and street address
  $js_file = drupal_get_path('module', 'ip_geoloc') . '/js/ip_geoloc_current_location.js';
  drupal_add_js($js_file, array('weight' => 1));
}

/**
 * Outputs an HTML div containing the formatted address reverse-geocoded from
 * the supplied lat,long coordinates.
 *
 * See the CALLER BEWARE note at ip_geoloc_reverse_geocode().
 *
 * @param $latitude,  string or double, e.g. "-37.87" or -37.87
 * @param $longitude, string or double, e.g. "144.98" or 144.98
 */
function ip_geoloc_get_address($latitude, $longitude) {
  $google_address = ip_geoloc_reverse_geocode($latitidue, $longitude);
  return isset($google_address['formatted_address']) ? $google_address['formatted_address'] : '';
}

/**
 * Uses the Google webservice to retrieve address information based on
 * supplied (lat, long) coordinates.
 * Effectively makes calls of this form:
 * http://maps.googleapis.com/maps/api/geocode/json?sensor=false&latlng=-37.87,144.98
 *
 * CALLER BEWARE:
 * This is a server-side call. If you want to call this function repeatedly,
 * remeber that Google imposes a limit of 2500 calls per 24hr and 20 calls per
 * second.
 * So for bulk address generation, put a sleep(1) before each repeat or you are
 * likely to face an OVER_QUERY_LIMIT response.
 *
 * @param $latitude,  string or double, e.g. "-37.87" or -37.87
 * @param $longitude, string or double, e.g. "144.98" or 144.98
 */
function ip_geoloc_reverse_geocode($latitude, $longitude) {
  if (empty($latitude) || empty($latitude)) {
    drupal_set_message(t('IP Geolocation: empty lat/long -- cannot reverse-geocode to address.'), 'warning');
    return FALSE;
  }
  $url = "http://maps.googleapis.com/maps/api/geocode/json?sensor=false&latlng=$latitude,$longitude";
  $response = drupal_http_request($url);
  if (!empty($response->error)) {
    drupal_set_message(t('IP Geolocation: the HTTP request %url returned the following error (code @code): %error',
      array('%url' => $url, '@code' => $response->code, '%error' => $response->error)), 'error');
    return FALSE;
  }
  $data = drupal_json_decode($response->data);
  if ($data['status'] == 'OVER_QUERY_LIMIT') {
    drupal_set_message(t('IP Geolocation: the request %url succeeded, but Google says you are over your query limit.',
      array('%url' => $url)), 'warning');
    return FALSE;
  }
  if ($data['status'] == 'ZERO_RESULTS' || !isset($data['results'][0])) {
    drupal_set_message(t('IP Geolocation: The @protocol request %url succeeded, but returned no results.',
      array('@protocol' => $response->protocol, '%url' => $url)), 'warning');
    return FALSE;
  }
  if ($data['status'] != 'OK') {
    drupal_set_message(t('IP Geolocation: something went wrong with request %url',
      array('%url' => $url)), 'error');
    return FALSE;
  }
  $google_address = $data['results'][0];
  if (empty($google_address['formatted_address'])) {
    ip_geoloc_debug(t('IP Geolocation: (@lat, @long) could not be reverse-geocoded to a street address.',
      array('@lat' => $latitude, '@long' => $longitude)), 'warning');
  }
  else {
    ip_geoloc_debug(t('IP Geolocation: (@lat, @long) was reverse-geocoded to %address',
      array('@lat' => $latitude, '@long' => $longitude, '%address' => $google_address['formatted_address'])));
  }
  return $google_address;
}
