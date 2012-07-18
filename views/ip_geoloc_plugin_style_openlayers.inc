<?php

/**
 * @file
 * ip_geoloc_plugin_style_openlayers.inc
 *
 * Views Style plugin extension for OpenLayers (if enabled).
 */

require_once 'ip_geoloc_plugin_style.inc';

class ip_geoloc_plugin_style_openlayers extends views_plugin_style {

  /**
   * Set default OpenLayer options.
   */
  function option_definition() {
    $options = parent::option_definition();

    if (empty($latitude)) {
      $latitude  = module_exists('location') ? 'location_latitude' : 'ip_geoloc_latitude';
    }
    if (empty($longitude)) {
      $longitude = module_exists('location') ? 'location_longitude' : ($latitude == 'ip_geoloc_latitude' ? 'ip_geoloc_longitude' : $latitude); // for field-based modules
    }
    $options['map'] = array('default' => variable_get('openlayers_default_map', 'default'));
    $options['ip_geoloc_views_plugin_latitude']  = array('default' => $latitude);
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => $longitude);
    $options['ip_geoloc_views_plugin_center_option'] = array('default' => IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['map'] = array(
      '#type' => 'select',
      '#title' => t('Map'),
      '#description' => t('The OpenLayers map used to place the view locations on. You can configure map and markers <a href="/admin/structure/openlayers/maps">here</a>.'),
      '#options' => openlayers_map_options(),
      '#default_value' => $this->options['map'] ? $this->options['map'] : variable_get('openlayers_default_map', 'default'),
    );
    $form['ip_geoloc_views_plugin_latitude'] = array(
      '#title' => t('Name of latitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_latitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_latitude</strong>, unless your view involves other modules or relationships, in which case you may have to prefix the name with the relevant table name(s), delimited by underscores. For instance, if you use a view based on the <strong>Location</strong> module, use <strong>location_latitude</strong>. If the latitude is stored in a <strong>text field</strong>, then enter the field\'s machine name, e.g. when using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules enter <em>field_myname</em>. Your field name will be on <a href="/admin/reports/views-fields">this page</a>.')
    );
    $form['ip_geoloc_views_plugin_longitude'] = array(
      '#title' => t('Name of longitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_longitude'],
      '#description' => t('See comments above. When using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules you can leave this field empty.')
    );
    $form['ip_geoloc_views_plugin_center_option'] = array(
      '#title' => t('Map centering options'),
      '#type' => 'radios',
      '#default_value' => $this->options['ip_geoloc_views_plugin_center_option'],
      '#options' => array(
        IP_GEOLOC_MAP_CENTER_FIXED => t('Fixed center (see note 1).'),
        IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION => t('Use the first location returned by the view as the center of the map.'),
        IP_GEOLOC_MAP_CENTER_ON_VISITOR => t("Center the map on the visitor's current location, optionally with a special marker (see note 2)"),
      ),
      '#description' => t('<strong>Note 1 </strong> If you choose the first option, the map may be centered via its "Center & Bounds" options. Click the Edit link (if available) for the map you selected on <a href="/admin/structure/openlayers/maps">this page</a>.<br/><strong>Note 2 </strong>A similar option is available natively as an OpenLayers "Behavior". However, in case it fails, it does not fall back on an IP address based position lookup. It also does not feature the display of the visitor\'s position as an additional marker on the map.')
    );

  }

  /**
   * Transform the View result in a list of marker locations and render these on
   * a map.
   *
   * @param type $result
   *   This parameter appears to be superfluous, same as $this->view->result.
   */
  function render($result) {

    if (!empty($this->view->live_preview)) {
    //return t('The preview function is incompatible with the map format so cannot be used. Please visit the page path or block to see the results.');
    }

    // This somewhat mimics function openlayers_render_map_data() and class
    // openlayers_views_style_map, function render()
    $map = openlayers_map_load($this->options['map']);

    // function openlayers_build_map() invokes:
    // o hook_openlayers_map_preprocess_alter(&$map), which introduces the names
    //   of two layers we'll use: IP_GEOLOC_MARKER_LAYER and IP_GEOLOC_VISITOR_MARKER_LAYER
    // o hook_openlayers_layers(), which creates barebone layers for the above
    // o hook_openlayers_map_alter(&$map), for modules to make final changes to
    //   the map and its layers
    $map = openlayers_build_map($map->data);

    // Return themed map if no errors found.
    if (empty($map['errors'])) {

      ip_geoloc_plugin_style_render_fields($this);
      $locations = ip_geoloc_plugin_style_extract_locations($this);

      $center_option = !isset($this->options['ip_geoloc_views_plugin_center_option']) ? IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION : $this->options['ip_geoloc_views_plugin_center_option'];

      if (!isset($map['layers'][IP_GEOLOC_MARKER_LAYER]['features'])) {
        $map['layers'][IP_GEOLOC_MARKER_LAYER]['features'] = array();
      }
      foreach ($locations as $location) {
        if (empty($map['layers'][IP_GEOLOC_MARKER_LAYER]['features'])) { // first location
          if ($center_option == IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION) {
            $map['center']['initial']['centerpoint'] = $location->longitude . ',' . $location->latitude;
          }
          elseif ($center_option >= IP_GEOLOC_MAP_CENTER_ON_VISITOR && isset($_SESSION['ip_geoloc']['location']['longitude'])) {
            $longitude = $_SESSION['ip_geoloc']['location']['longitude'];
            $latitude  = $_SESSION['ip_geoloc']['location']['latitude'];
            $map['center']['initial']['centerpoint'] = "$longitude,$latitude";
          }
        }
        $feature = array(
          // Only 'name' and 'description' are valid attribute names, see openlayers_behavior_popup.js
          'attributes' => array('description' => $location->balloonText),
          'projection' => '4326',
          'wkt' => 'POINT(' . $location->longitude . ' ' . $location->latitude . ')');
        $map['layers'][IP_GEOLOC_MARKER_LAYER]['features'][] = $feature;
      }

      // See admin/structure/openlayers/maps/<map-name>/edit...
      // If "Hide empty map for Views" is ticked, then if the map has no
      // features, it shouldn't be displayed.
      if (!empty($map['hide_empty_map'])) {
        if (empty($map['layers'][IP_GEOLOC_MARKER_LAYER]['features']) && empty($map['layers'][IP_GEOLOC_VISITOR_MARKER_LAYER]['features'])) {
          return '';
        }
      }

      $js = array('openlayers' => array('maps' => array($map['id'] => $map)));
      drupal_add_js($js, 'setting');

      // Unlike openlayers.theme.inc, theme_openlayers_map(), we use a template.
      //
      // To ensure that any controls are on the map correctly, we need to
      // wrap the map in a container, and take into account %-dimensions
      $container_width = $map['width'];
      $container_height = $map['height'];
      $map['width']  = (strpos($map['width'],  '%') > 0) ? '100%' : $map['width'];
      $map['height'] = (strpos($map['height'], '%') > 0) ? '100%' : $map['height'];

      $output = theme(array('ip_geoloc_openlayers'), array( // ip-geoloc-openlayers.tpl.php
        'view' => $this->view,
      //'options' => $this->options,
        'map' => $map,
        'container_width' => $container_width,
        'container_height' => $container_height
      ));
    }
    else {
      $output = implode('<br/>', $map['errors']);
    }
    return $output;
  }
}
