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
    $options['default_marker_color'] = array('default' => '');
    $options['center_option'] = array('default' => IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION);
    $options['zoom'] = array('default' => 2);
    $options['visitor_marker_color'] = array('default' => 'none');
    $options['map'] = array('default' => '');
    $options['map_height'] = array('default' => 400);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form_state['renderer'] = 'leaflet';
    ip_geoloc_plugin_style_bulk_of_form($this, $form, $form_state);

    $form['center_option']['#options'][0] = t('Auto-box');
    unset($form['center_option']['#description']);

    $form['zoom'] = array(
      '#title' => t('Zoom level (1..18)'),
      '#type' => 'textfield',
      '#size' => 2,
      '#default_value' => $this->options['zoom'],
      '#description' => t('Does not apply when the auto-box option is selected.')
    );

    $visitor_marker_colors = array(
      'none' => '<' . t('none') . '>') +
      ip_geoloc_marker_colors();

    $form['visitor_marker_color'] = array(
      '#title' => t('Visitor marker color'),
      '#type' => 'select',
      '#multiple' => FALSE,
      '#default_value' => $this->options['visitor_marker_color'],
      '#options' => $visitor_marker_colors,
      '#description' => t(''),
      '#attributes' => array('class' => array('marker-color gmap')),
      '#attached' => array('css' => array(drupal_get_path('module', 'ip_geoloc') . '/ip_geoloc.css')
      )
    );

    $maps = array();
    foreach (ip_geoloc_plugin_style_leaflet_map_get_info() as $key => $map) {
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
      '#description' => t('The default produces a map of 400 pixels high with a width extending to its bounding container.')
    );
  }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
    if (!is_numeric($form_state['values']['style_options']['zoom']) || $form_state['values']['style_options']['zoom'] < 0) {
      form_error($form['zoom'], t('Zoom level must be a positive number.'));
    }
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

    if (empty($this->options['map']) || !($map = ip_geoloc_plugin_style_leaflet_map_get_info($this->options['map']))) {
      return t('No map selected or map data not found.');
    }

    if (!empty($this->view->live_preview)) {
      return t('The preview function is incompatible with Leaflet maps so cannot be used. Please visit the page path or the block to view your map.');
    }

    ip_geoloc_plugin_style_render_fields($this);
    $locations = ip_geoloc_plugin_style_extract_locations($this);

    $marker_color = $this->options['default_marker_color'];
    $center_option = !isset($this->options['center_option']) ? IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION : $this->options['center_option'];
    $zoom = $this->options['zoom'];
    $visitor_marker_color = $this->options['visitor_marker_color'];

    $icon_path = variable_get('ip_geoloc_marker_directory', drupal_get_path('module', 'ip_geoloc') . '/markers');
    // Need to prepend a slash when contextual filter is present in URL
    // Does not work for blocks
    if (isset($this->display->display_options['path'])) {
      $view_path = $this->display->display_options['path'];
      $arg = drupal_substr($_GET['q'], strlen($view_path));
      if (strlen($arg) > 1 && drupal_substr($arg, 0, 1) == '/') {
        $icon_path = '/' . $icon_path;
      }
    }

    if ($center_option == IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION && !empty($locations)) {
      $first_location = reset($locations);
      $map['center'] = array(
        'lat' => $first_location->latitude,
        'lon' => $first_location->longitude
      );
      $map['settings']['zoom'] = $zoom;
    }
    elseif ($center_option >= IP_GEOLOC_MAP_CENTER_ON_VISITOR && isset($_SESSION['ip_geoloc']['location']['latitude'])) {
      $map['center'] = array(
        'lat' => $_SESSION['ip_geoloc']['location']['latitude'],
        'lon' => $_SESSION['ip_geoloc']['location']['longitude']
      );
      $map['settings']['zoom'] = $zoom;
    }
    else {
      // Auto-box
    }

    $features = array();
    if ($visitor_marker_color != 'none' && isset($_SESSION['ip_geoloc']['location']['latitude'])) {
      // See leaflet/README.txt for examples of Leaflet "features"
      $visitor_feature = array(
        'type' => 'point',
        'lat' => $_SESSION['ip_geoloc']['location']['latitude'],
        'lon' => $_SESSION['ip_geoloc']['location']['longitude'],
        'popup' => t('Your retrieved location'),
      );
      if ($visitor_marker_color != '') {
        $visitor_feature['icon'] = array(
          'iconUrl' => $icon_path . "/$visitor_marker_color.png",
          'iconSize' => array('x' => 21, 'y' => 34), // width, height
          'iconAnchor' => array('x' => 11, 'y' => 34), // center baseline
          'popupAnchor' => array('x' => 0, 'y' => -35), // above topline, center
        );
      }
      $features[] = $visitor_feature;
    }

    foreach ($locations as $key => $location) {

      $feature = array(
        'type' => 'point',
        'lat' => $location->latitude,
        'lon' => $location->longitude,
        'popup' => $location->balloon_text,
      //'leaflet_id' => $key
      );
      if (!empty($location->marker_color) || !empty($marker_color)) {
        // Switch from default icon
        $color = empty($location->marker_color) ? $marker_color : $location->marker_color;
        $feature['icon'] = array(
          'iconUrl' => $icon_path . "/$color.png",
          'iconSize' => array('x' => 21, 'y' => 34), // width, height
          'iconAnchor' => array('x' => 11, 'y' => 34), // center baseline
          'popupAnchor' => array('x' => 0, 'y' => -35), // above topline, center
        //'shadowUrl'
        //'shodowSize'
        //'shadowAnchor
        );
      }
      $features[] = $feature;
    }

    $map_id = 'ip-geoloc-map-of-view-' . $this->view->name . '-' . $this->display->id;

    //ip_geoloc_debug(print_r($map, TRUE));
    //ip_geoloc_debug(print_r($features, TRUE));

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
 *
 * Note: this indirectly calls ip_geoloc_leaflet_map_info_alter($map_info).
 */
function ip_geoloc_plugin_style_leaflet_map_get_info($map_name = NULL) {
  return leaflet_map_get_info($map_name);
}
