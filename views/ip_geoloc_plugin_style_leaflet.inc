<?php

/**
 * @file
 * ip_geoloc_plugin_style_leaflet.inc
 *
 * Views Style plugin extension for Leaflet (if enabled).
 */
require_once 'ip_geoloc_plugin_style.inc';

class ip_geoloc_plugin_style_leaflet extends views_plugin_style {

  /**
   * Set default OpenLayer options.
   */
  function option_definition() {
    $options = parent::option_definition();

    $latitude  = module_exists('location') ? 'location_latitude' : 'ip_geoloc_latitude';
    $longitude = module_exists('location') ? 'location_longitude' : ($latitude == 'ip_geoloc_latitude' ? 'ip_geoloc_longitude' : $latitude); // for field-based modules
    $options['ip_geoloc_views_plugin_latitude' ] = array('default' => $latitude);
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => $longitude);

    $options['map'] = array('default' => '');
    $options['map_height'] = array('default' => '400');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    ip_geoloc_plugin_style_bulk_of_form($this, $form, $form_state, $short_form = TRUE);

    $maps = array();
    foreach (ip_geoloc_map_get_info() as $key => $map) {
      $maps[$key] = t($map['label']);
    }
    $form['map'] = array(
      '#title' => t('Map'),
      '#type' => 'select',
      '#options' => $maps,
      '#default_value' => isset($this->options['map']) ? $this->options['map'] : '',
      '#required' => TRUE,
    );

    $form['map_height'] = array(
      '#title' => t('Map height'),
      '#type' => 'textfield',
      '#field_suffix' => t('px'),
      '#size' => 4,
      '#default_value' => $this->options['map_height'],
      '#required' => FALSE,
      '#description' => t('The default produces a map of 400 pixels high with a width extending to its bounding container.')
    );
  }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
    if (!is_numeric($form_state['values']['style_options']['map_height']) || $form_state['values']['style_options']['map_height'] < 0) {
      form_error($form['map_height'], t('Map height must be a positive number.'));
    }
  }

  /**
   * Transform the View result in a list of marker locations and render on map.
   *
   * @param type $result
   *   This parameter appears to be superfluous, same as $this->view->result.
   */
  function render($result) {

    if (empty($this->options['map']) || !($map = ip_geoloc_map_get_info($this->options['map']))) {
      return(t('No map selected or map data not found.'));
    }

    if (!empty($this->view->live_preview)) {
      return t('The preview function is incompatible with Leaflet maps so cannot be used. Please visit the page path or the block to view your map.');
    }

    ip_geoloc_plugin_style_render_fields($this);
    $locations = ip_geoloc_plugin_style_extract_locations($this);

    // See leaflet/README.txt for examples of Leaflet "features"
    $icon_path = drupal_get_path('module', 'ip_geoloc') . '/markers/';
    $features = array();
    foreach ($locations as $key => $location) {
      $feature = array(
        'type' => 'point',
        'lat' => $location->latitude,
        'lon' => $location->longitude,
        'popup' => $location->balloon_text,
      //'leaflet_id' => $key
      );
      if (!empty($location->marker_color)) {
      // Doesn't seem to work
      // $feature['icon'] = array('iconUrl' => $icon_path . $location->marker_color . '.png');
      }
      $features[] = $feature;
    }

    $map_id = 'leaflet_map_' . md5(serialize($features) . serialize($features));

    drupal_add_js(drupal_get_path('module', 'leaflet') . '/leaflet.drupal.js');
    $settings = array(
      'mapId' => $map_id,
      'map' => $map,
      'features' => $features,
    );
    drupal_add_js(array('leaflet' => array($settings)), 'setting');
    drupal_add_library('leaflet', 'leaflet');

    $output = theme('ip_geoloc_leaflet', array(
      'map_id' => $map_id,
      'height' => $this->options['map_height'],
      'view' => $this->view
    ));
    return $output;
  }
}

/**
 * Wrapper around the only programmatic dependency we have on the Leaflet module.
 */
function ip_geoloc_map_get_info($map_name = NULL) {
  return leaflet_map_get_info($map_name);
}