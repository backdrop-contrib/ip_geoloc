<?php

/**
 * @file
 * Blocks available in IP Geolocation.
 */

/**
 * Implements hook_block_info().
 */
function ip_geoloc_block_info() {
  $blocks['current_visitor_map'] = array(
    'info' => t("Map centered on current visitor's location"),
  );
  $blocks['recent_visitors_map'] = array(
    'info' => t('Map showing locations of @n most recent visitors', array('@n' => variable_get('ip_geoloc_recent_visitors_map_number_of_visitors', 10))),
  );
  return $blocks;
}

/**
 * Implements hook_block_configure().
 */
function ip_geoloc_block_configure($delta = '') {
  $form = array();
  if ($delta == 'recent_visitors_map') {
    $form['ip_geoloc_recent_visitors_map_number_of_visitors'] = array(
      '#type' => 'textfield',
      '#title' => t('Number of recent visitor locations to show on the map.'),
      '#default_value' => variable_get('ip_geoloc_recent_visitors_map_number_of_visitors', 10),
      '#description' => t('Must be greater than 0. Values greater than 20 are not recommended.')
    );
  }
  $form['ip_geoloc_' . $delta . '_div_style'] = array(
    '#type' => 'textfield',
    '#title' => t('CSS style attribute(s) applied to the HTML DIV element that is placeholding the map'),
    '#default_value' => variable_get('ip_geoloc_' . $delta . '_div_style', IP_GEOLOC_MAP_DIV_DEFAULT_STYLE),
    '#description' => t('The default %default_style results in a map of 300 pixels high, with a width bounded by the element that contains it. Do not enter quotes or equal signs and make sure your quotes match.',
      array('%default_style' => IP_GEOLOC_MAP_DIV_DEFAULT_STYLE))
  );
  if ($delta == 'current_visitor_map') {
    $form['ip_geoloc_current_visitor_map_options'] = array(
      '#type' => 'textfield',
      '#size' => 120,
      '#title' => t('Map options'),
      '#default_value' => variable_get('ip_geoloc_current_visitor_map_options', IP_GEOLOC_CURRENT_VISITOR_MAP_OPTIONS),
      '#description' => t("The default %default_options produces a street map zoomed in to level 15. A list of map options can be found <a href='!google_map_docs'>here</a>. Remember to separate options with comma's, not semi-colons, and make sure your quotes match.",
            array('%default_options' => IP_GEOLOC_CURRENT_VISITOR_MAP_OPTIONS,
                  '!google_map_docs' => DOC_GOOGLE_MAP_OPTIONS))
    );
  }
  elseif ($delta == 'recent_visitors_map') {
    $form['ip_geoloc_recent_visitors_map_options'] = array(
      '#type' => 'textfield',
      '#size' => 120,
      '#title' => t('Map options'),
      '#default_value' => variable_get('ip_geoloc_recent_visitors_map_options', IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS),
      '#description' => t("The default %default_options produces a world map zoomed in to level 2. A list of map options can be found <a href='!google_map_docs'>here</a>. Remember to separate options with comma's, not semi-colons, and make sure your quotes match.",
            array('%default_options' => IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS,
                  '!google_map_docs' => DOC_GOOGLE_MAP_OPTIONS))
    );
  }
  return $form;
}

/**
 * Implements hook_block_save().
 */
function ip_geoloc_block_save($delta = '', $edit = array()) {
  variable_set('ip_geoloc_' . $delta . '_div_style', $edit['ip_geoloc_' . $delta . '_div_style']);
  if ($delta == 'current_visitor_map') {
    ip_geoloc_save_map_options('ip_geoloc_current_visitor_map_options', $edit['ip_geoloc_current_visitor_map_options']);
  }
  elseif ($delta == 'recent_visitors_map') {
    ip_geoloc_save_map_options('ip_geoloc_recent_visitors_map_options', $edit['ip_geoloc_recent_visitors_map_options']);
    variable_set('ip_geoloc_recent_visitors_map_number_of_visitors', $edit['ip_geoloc_recent_visitors_map_number_of_visitors']);
  }
}

function ip_geoloc_save_map_options($var_name, $map_options) {
  $map_options_decoded = drupal_json_decode($map_options);
  if ($map_options_decoded == NULL) {
    drupal_set_message(t("Sytax error in map options. These map options may not work: '%map_options'", array('%map_options' => $map_options)), 'warning');
  }
  variable_set($var_name, $map_options);
}

/**
 * Implements hook_block_view().
 */
function ip_geoloc_block_view($delta = '') {
  $block = array();
  $map_style = variable_get('ip_geoloc_' . $delta . '_div_style', IP_GEOLOC_MAP_DIV_DEFAULT_STYLE);
  switch ($delta) {
    case 'current_visitor_map':
      $block['subject'] = t('Your location');
      $map_options = variable_get('ip_geoloc_current_visitor_map_options', IP_GEOLOC_CURRENT_VISITOR_MAP_OPTIONS);
      $block['content'] = ip_geoloc_output_map_current_location('ip-geoloc-block-current-visitor-map', drupal_json_decode($map_options), $map_style);
      break;

    case 'recent_visitors_map':
      $block['subject'] = t('Recent visitors');
      $how_many = variable_get('ip_geoloc_recent_visitors_map_number_of_visitors', 10);
      if ($how_many > 0) {
        $locations = ip_geoloc_get_recent_visitor_locations($how_many);
        $map_options = variable_get('ip_geoloc_recent_visitors_map_options', IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS);
        $block['content'] = ip_geoloc_output_map_multi_locations($locations, 'ip-geoloc-block-recent-visitors-map', drupal_json_decode($map_options), $map_style);
      }
      break;
  }
  return $block;
}

function ip_geoloc_get_recent_visitor_locations($how_many) {
  $result = db_query_range("SELECT DISTINCT ip_address, latitude, longitude, formatted_address, COUNT(a.timestamp) AS visit_count, MAX(a.timestamp) AS last_visit FROM {ip_geoloc} i INNER JOIN {accesslog} a ON i.ip_address = a.hostname GROUP BY a.hostname ORDER BY last_visit DESC", 0, $how_many);
  $locations = array();
  $date_format = variable_get('date_format_short', 'm/d/Y - H:i');
  foreach ($result as $location) {
    if (!isset($locations[$location->ip_address])) {
      $location->last_visit = date($date_format, $location->last_visit);
      $locations[$location->ip_address] = $location;
    }
  }
  return $locations;
}
