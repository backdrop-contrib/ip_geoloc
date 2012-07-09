<?php

/**
 * @file
 * ip_geoloc_plugin_style_openlayers.inc
 *
 * Views Style plugin extension for OpenLayers (if enabled).
 */

class ip_geoloc_plugin_style_openlayers extends views_plugin_style {

  /**
   * Set default OpenLayer options.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['map'] = array('default' => variable_get('openlayers_default_map', 'default'));
    $options['ip_geoloc_views_plugin_latitude']  = array('default' => 'ip_geoloc_latitude');
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => 'ip_geoloc_longitude');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['map'] = array(
      '#type' => 'select',
      '#title' => t('Map'),
      '#description' => t('The OpenLayers "canvas" used to render the view.'),
      '#options' => openlayers_map_options(),
      '#default_value' => $this->options['map'] ? $this->options['map'] : variable_get('openlayers_default_map', 'default'),
    );
    $form['ip_geoloc_views_plugin_latitude'] = array(
      '#title' => t('Name of latitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_latitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_latitude</strong>, unless you are viewing another module or a relationship, in which case you may have to prefix this name with the relevant table name(s), delimited by underscores. For instance, if you use a view based on the <strong>Location</strong> module, use <strong>location_latitude</strong>. If the latitude is stored in a <strong>text field</strong>, then enter the field\'s machine name, e.g. when using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules enter <em>field_myname</em>. Valid field names are on <a href="/admin/reports/views-fields">this page</a>.')
    );
    $form['ip_geoloc_views_plugin_longitude'] = array(
      '#title' => t('Name of longitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_longitude'],
      '#description' => t('See comments above. When using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules you can leave this field empty.')
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
      return t('The preview function is incompatible with the map format so cannot be used. Please visit the page path or block to see the results.');
    }


    // This somewhat mimics function openlayers_render_map_data() and class
    // openlayers_views_style_map, function render()
    $map = openlayers_map_load($this->options['map']);

    // function openlayers_build_map() invokes:
    // o hook_openlayers_map_preprocess_alter(&$map), which we use to set the
    //   center of any OpenLayers map, even when not created through this View
    // o hook_openlayers_layers(), which creates an empty layer by the name of
    //   IP_GEOLOC_MARKER_LAYER
    // o hook_openlayers_map_alter(&$map), for modules to make final changes to
    //   the map and its layers
    $map = openlayers_build_map($map->data);
    $layer_name = IP_GEOLOC_MARKER_LAYER;

    // Return themed map if no errors found.
    if (empty($map['errors'])) {

      include_once 'ip_geoloc_plugin_style.inc';
      ip_geoloc_plugin_style_render_fields($this);
      $locations = ip_geoloc_plugin_style_extract_locations($this);

      if (!isset($map['layers'][$layer_name]['features'])) {
        $map['layers'][$layer_name]['features'] = array();
      }
      foreach ($locations as $location) {
        $feature = array(
          'attributes' => array('title_rendered' => $location->balloonText),
          'projection' => '4326',
          'wkt' => 'POINT(' . $location->longitude . ' ' . $location->latitude . ')');
        $map['layers'][$layer_name]['features'][] = $feature;
      }
      //drupal_set_message(var_dump($map));

      // See admin/structure/openlayers/maps/<mapname>/edit...
      // If "Hide empty map for Views" is ticked, then if the map has no
      // features, it shouldn't be displayed.
      if (!empty($map['hide_empty_map'])) {
        if (empty($map['layers'][$layer_name]['features'])) {
          return '';
        }
      }

      $js = array('openlayers' => array('maps' => array($map['id'] => $map)));
      drupal_add_js($js, 'setting');

      // Unlike openlayers.theme.inc theme_openlayers_map(); we use a template.
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
