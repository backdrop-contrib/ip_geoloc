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

    $options['differentiator'] = array('contains' => array(
      'differentiator_field' => array('default' => '')
    ));
    $options['default_marker_color'] = array('default' => '');
    $options['center_option'] = array('default' => IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION);
    $options['zoom'] = array('default' => 2);
    $options['visitor_marker_color'] = array('default' => 'none');
    $options['map'] = array('default' => '');
    $options['map_height'] = array('default' => 300);
    $options['cluster_radius'] = array('default' => module_exists('leaflet_markercluster') ? 80 : '');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form_state['renderer'] = 'leaflet';
    ip_geoloc_plugin_style_bulk_of_form($this, $form, $form_state);

    $form['center_option']['#options'][0] = t('Auto-box (include visitor marker if color <strong>not</strong> set to &lt;none&gt;)');
  //$form['center_option']['#options'][IP_GEOLOC_MAP_AUTOBOX_IGNORE_VISITOR] = t('Auto-box (ignore visitor location)');
    unset($form['center_option']['#description']);

    $form['zoom'] = array(
      '#title' => t('Initial zoom level (0..18)'),
      '#type' => 'textfield',
      '#size' => 2,
      '#default_value' => $this->options['zoom'],
      '#description' => t('Does not apply to auto-box centering except when only one or no markers are shown.')
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
      '#attached' => array('css' => array(drupal_get_path('module', 'ip_geoloc') . '/css/ip_geoloc_admin.css')
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
      '#description' => t('The default produces a map of 300 pixels high with a width extending to its bounding container.')
    );

    $form['cluster_radius'] = array(
      '#title' => t('Cluster marker radius'),
      '#type' => 'textfield',
      '#field_suffix' => t('px'),
      '#size' => 4,
      '#default_value' => $this->options['cluster_radius'],
      '#description' => module_exists('leaflet_markercluster') ? t('Leave empty to disable clustering.') : t('Requires the <a href="!url">Leaflet MarkerCluster</a> module and corresponding javascript library.', array(
        '!url' => url('http://drupal.org/project/leaflet_markercluster')
      ))
    );
  }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
    if ($form_state['values']['style_options']['zoom'] != '' && (!is_numeric($form_state['values']['style_options']['zoom']) || $form_state['values']['style_options']['zoom'] < 0)) {
      form_error($form['zoom'], t('Zoom level must be a non-negative number.'));
    }
    if (!is_numeric($form_state['values']['style_options']['map_height']) || $form_state['values']['style_options']['map_height'] <= 0) {
      form_error($form['map_height'], t('Map height cannot be blank and must be a positive number.'));
    }
  }

  /**
   * Transform the View result in a list of marker locations and render on map.
   */
  function render() {

    if (empty($this->options['map']) || !($map = ip_geoloc_plugin_style_leaflet_map_get_info($this->options['map']))) {
      return t('No Leaflet map was selected or map configuration was not found.');
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
    $cluster_radius = $this->options['cluster_radius'];

    $icon_path = variable_get('ip_geoloc_marker_directory', drupal_get_path('module', 'ip_geoloc') . '/markers');
    $icon_path = file_create_url($icon_path);

    if (!empty($locations) &&
        ($center_option == IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION ||
        ($visitor_marker_color == 'none' && count($locations) == 1))) {
      $first_location = reset($locations);
      $map['center'] = array(
        'lat' => $first_location->latitude,
        'lon' => $first_location->longitude
      );
    }
    elseif (($center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS || $center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS_WEIGHTED) && !empty($locations)) {
      list($center_lat, $center_lon) = ip_geoloc_center_of_locations($locations, $center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS_WEIGHTED);
      $map['center'] = array(
        'lat' => $center_lat,
        'lon' => $center_lon
      );
    }

    uasort($locations, '_ip_geoloc_plugin_style_leaflet_compare');

    $visitor_location = ip_geoloc_get_visitor_location();
    if ((empty($locations) || $center_option == IP_GEOLOC_MAP_CENTER_ON_VISITOR) && isset($visitor_location['latitude'])) {
      $map['center'] = array(
        'lat' => $visitor_location['latitude'],
        'lon' => $visitor_location['longitude']
      );
    }
    $marker_dimensions = explode('x', variable_get('ip_geoloc_marker_dimensions', '21 x 34'));
    $marker_width  = (int)$marker_dimensions[0];
    $marker_height = (int)$marker_dimensions[1];

    $features = array();
    if ($visitor_marker_color != 'none' && isset($visitor_location['latitude'])) {
      // See leaflet/README.txt for examples of Leaflet "features"
      $visitor_feature = array(
        'type' => 'point',
        'lat' => $visitor_location['latitude'],
        'lon' => $visitor_location['longitude'],
        'popup' => t('Your retrieved location'),
      );
      if ($visitor_marker_color != '') {
        $visitor_feature['icon'] = array(
          'iconUrl' => $icon_path . "/$visitor_marker_color.png",
          'iconSize' => array('x' => $marker_width, 'y' => $marker_height),
          'iconAnchor' => array('x' => (int)(($marker_width + 1) / 2), 'y' => $marker_height), // center of baseline
          'popupAnchor' => array('x' => 0, 'y' => -$marker_height - 1), // just above topline, center
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
          'iconSize' => array('x' => $marker_width, 'y' => $marker_height), // width, height
          'iconAnchor' => array('x' => (int)(($marker_width + 1) / 2), 'y' => $marker_height), // center baseline
          'popupAnchor' => array('x' => 0, 'y' => -$marker_height - 1), // just above topline, center
        //'shadowUrl'
        //'shodowSize'
        //'shadowAnchor
        );
      }
      $features[] = $feature;
    }
    // If auto-box is chosen ($center_option==0), zoom only when there are 0 or 1 markers [#1863374]
    if (empty($center_option) && count($features) > 1) {
      unset($map['settings']['zoom']);
    }
    else {
      $map['settings']['zoom'] = $zoom;
    }

    if (!empty($cluster_radius)) {
      $map['settings']['maxClusterRadius'] = $cluster_radius;
    }
    $map['settings']['animateAddingMarkers'] = TRUE;

    $map_id = 'ip-geoloc-map-of-view-' . $this->view->name . '-' . $this->display->id . '-' . md5(serialize($features)); // [#1802732]

    drupal_add_js(drupal_get_path('module', 'leaflet') . '/leaflet.drupal.js');
    $settings = array(
      'mapId' => $map_id,
      'map' => $map,
      'features' => $features,
    );
    drupal_add_js(array('leaflet' => array($settings)), 'setting');
    drupal_add_library('leaflet', 'leaflet');

    if (!empty($cluster_radius)) {
      // This is hacky but can't see another way to conditionally load Leaflet
      // MarkerCluster library
      if (module_exists('leaflet_markercluster')) {
        $vars = array();
        leaflet_markercluster_preprocess_leaflet_map($vars);
      }
      else {
        drupal_set_message(t('Cannot cluster - Leaflet Markercluster module not enabled.'), 'warning');
      }
    }

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

function _ip_geoloc_plugin_style_leaflet_compare($location1, $location2) {
  $weight1 = empty($location1->weight) ? 0 : $location1->weight;
  $weight2 = empty($location2->weight) ? 0 : $location2->weight;
  return $weight2 - $weight1;
}
