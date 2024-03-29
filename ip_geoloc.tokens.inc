<?php


/**
 * Implements hook_token_info().
 */
function ip_geoloc_token_info() {
  $types['visitor-location'] = array(
    'name' => t('Visitor location'),
    'description' => t("Tokens related to the visitor's location"),
  );

  $token_keys = array(
    'country',
    'country_code',
    'region',
    'region_code',
    'city',
    'locality',
    'route',
    'street_number',
    'postal_code',
    'administrative_area_level_1',
    'formatted_address',
    'latitude',
    'longitude',
    'ip_address',
    'provider',
  );

  $token_info = array();
  foreach ($token_keys as $key) {
    $name = str_replace('_', ' ', $key);
    $token_info[$key] = array(
      'name' => $name,
      'description' => t("Visitor's @name", array('@name' => $name)),
    );
  }
  $token_info['provider']['description'] = t('Provider of location, for example <em>user</em> or <em>google</em>');

  return array(
    'types' => $types,
    'tokens' => array('visitor-location' => $token_info),
  );
}

/**
 * Implements hook_tokens().
 */
function ip_geoloc_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $config = config('ip_geoloc.settings');

  $replacements = array();
  if ($type == 'visitor-location') {
    $is_sanitize = !empty($options['sanitize']);
    $location = ip_geoloc_get_visitor_location();
    foreach ($tokens as $key => $original) {
      if (isset($location[$key])) {
        $replacements[$original] = $is_sanitize ? filter_xss_admin($location[$key]) : $location[$key];
      }
    }
    if (!empty($replacements['[visitor-location:formatted_address]'])) {
      $need_street = $config->get('ip_geoloc_return_address_street');
      $need_locality = $config->get('ip_geoloc_return_address_locality');
      $need_country = $config->get('ip_geoloc_return_address_country');
      $address = _ip_geoloc_custom_formatted_address($location, $need_street, $need_locality, $need_country);
      $replacements['[visitor-location:formatted_address]'] = $is_sanitize ? filter_xss_admin($address) : $address;
    }
  }
  return $replacements;
}
