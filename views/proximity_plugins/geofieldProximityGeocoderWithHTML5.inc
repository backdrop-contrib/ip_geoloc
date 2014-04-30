<?php

/**
 * @file
 * Contains geofieldProximityGeocoderWithHTML5.
 */

class geofieldProximityGeocoderWithHTML5 extends geofieldProximityGeocoder {

  public function getSourceValue($views_plugin) {
    $input = isset($views_plugin->value)
      ? trim($views_plugin->value['origin'])
      : $views_plugin->options['geofield_proximity_geocoder'];

    if (empty($input) || $input == 'me' || $input == t('me')) {
      $location = ip_geoloc_get_visitor_location();
      if (isset($location['latitude'])) {
        return array(
          'latitude' => $location['latitude'],
          'longitude' => $location['longitude']
        );
      }
      ip_geoloc_debug('IGPV&M proximity: visitor location temporarily not available.', 'warning');
      return FALSE;
    }
    return parent::getSourceValue($views_plugin);
  }
}