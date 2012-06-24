<?php

define('IP_GEOLOC_MAP_NO_CENTER_OVERRIDE', 0);
define('IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION', 1);
define('IP_GEOLOC_MAP_CENTER_ON_VISITOR', 2);
define('IP_GEOLOC_MAP_CENTER_ON_VISITOR_IP', 3);

/**
 * @file
 * ip_geoloc_plugin_style_map.inc
 *
 * Views Style plugin extension.
 */
class ip_geoloc_plugin_style_map extends views_plugin_style {

  /**
   * Set default map options.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['ip_geoloc_map_of_view_div_style']  = array('default' => '');
    $options['ip_geoloc_map_of_view_options']    = array('default' => '');
    $options['ip_geoloc_views_plugin_latitude']  = array('default' => 'ip_geoloc_latitude');
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => 'ip_geoloc_longitude');
    $options['ip_geoloc_views_plugin_center_option'] = array('default' => IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['ip_geoloc_map_of_view_options'] = array(
      '#title' => t('Map options'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_map_of_view_options'],
      '#description' => t("If left blank, the default %default_options will produce a world map zoomed in to level 2. A list of map options can be found <a href='!google_map_docs'>here</a>. Remember to separate options with comma's, not semi-colons, and make sure your (double) quotes match.", array(
        '%default_options' => IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS,
        '!google_map_docs' => DOC_GOOGLE_MAP_OPTIONS))
    );
    $form['ip_geoloc_map_of_view_div_style'] = array(
      '#title' => t('Map style (CSS attributes)'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_map_of_view_div_style'],
      '#description' => t('If left blank, the default %default_style will result in a map of 300 pixels high, with a width bounded by the element that contains it. Separate style settings with semi-colons. Do not enter quotes or equal signs.',
        array('%default_style' => IP_GEOLOC_MAP_DIV_DEFAULT_STYLE))
    );
    $form['ip_geoloc_views_plugin_latitude'] = array(
      '#title' => t('Name of latitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_latitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_latitude</strong>, unless you are viewing another module or a relationship, in which case you may have to prefix this name with the relevant table name(s), delimited by underscores. For instance, if you use a view based on the <strong>Location</strong> module, use <strong>location_latitude</strong>. If the latitude is stored in a <strong>text field</strong>, then enter the field\'s machine name, e.g. when using the <strong>Geofield</strong> module enter <strong>field_myname</strong>')
    );
    $form['ip_geoloc_views_plugin_longitude'] = array(
      '#title' => t('Name of longitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_longitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_longitude</strong>, unless you are viewing another module or a relationship, in which case you may have to prefix this name with the relevant table name(s), delimited by underscores. For instance, if you use a view based on the <strong>Location</strong> module, use <strong>location_longitude</strong>. If the longitude is stored in a <strong>text field</strong>, then enter the field\'s machine name here, e.g. when using the <strong>Geofield</strong> module enter what you entered above for latitude.')
    );
    $form['ip_geoloc_views_plugin_center_option'] = array(
      '#title' => t('Map centering options'),
      '#type' => 'radios',
      '#default_value' => $this->options['ip_geoloc_views_plugin_center_option'],
      '#options' => array(
        IP_GEOLOC_MAP_NO_CENTER_OVERRIDE => t('No center override or fixed center (see note below).'),
        IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION => t('Use the first location returned by the view as the center of the map.'),
        IP_GEOLOC_MAP_CENTER_ON_VISITOR => t("Center the map on a distinct marker depicting the visitor's current location provided via their browser. This is subject to them accepting the prompt. If they decline, fall back on the next option."),
        IP_GEOLOC_MAP_CENTER_ON_VISITOR_IP => t("As above, however the visitor's location is taken from an IP address database lookup, which is less accurate."),
      ),
      '#description' => t('If you choose the first option you may center the map via the special <strong>Map options</strong> <em>"centerLat"</em> and <em>"centerLng"</em> e.g. %center_example', array(
        '%center_example' => '{ "mapTypeId":"roadmap", "centerLat":-38, "centerLng":145 }'))
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

    $latitude  = trim($this->options['ip_geoloc_views_plugin_latitude']);
    $longitude = trim($this->options['ip_geoloc_views_plugin_longitude']);

    if (empty($latitude)) {
      $latitude = 'ip_geoloc_latitude';
    }
    if (empty($longitude)) {
      $longitude = ($latitude == 'ip_geoloc_latitude') ? 'ip_geoloc_longitude' : $latitude; // for Geofield
    }

    // Do token substitutions, render timestamps etc, all the pre-theming stuff.
    // Exclude lat/long from this.
    unset($this->view->field[$latitude]);
    unset($this->view->field[$longitude]);
    $this->render_fields($result);

    $locations = array();
    foreach ($result as $i => $row) {
      $location = new stdClass();
      $row_has_location = TRUE;
      // First look for $latitude and $longitude as node field values:
      if (!empty($row->{'field_' . $latitude}[0]['raw']) && !empty($row->{'field_' . $longitude}[0]['raw'])) {

        // When a View is based on Geofield module lat/long are in
        // field_<field_name>[0]['raw']['lat'] and field_<field_name>[0]['raw']['lon']
        if (isset($row->{'field_' . $latitude}[0]['raw']['lat'])) {
          $location->latitude  = $row->{'field_' . $latitude }[0]['raw']['lat'];
          $location->longitude = $row->{'field_' . $longitude}[0]['raw']['lon'];
        }
        else {
          // Otherwise field values tend to be inside [0]['raw']['value']:
          $location->latitude  = $row->{'field_' . $latitude }[0]['raw']['value'];
          $location->longitude = $row->{'field_' . $longitude}[0]['raw']['value'];
        }
      }
      elseif (!empty($row->{$latitude}) && !empty($row->{$longitude})) {
        // Standard table fields: values are $row->{$latitude} and $row->{$longitude}
        $location->latitude  = $row->{$latitude};
        $location->longitude = $row->{$longitude};
      }
      else {
        $row_has_location = FALSE;
      }
      if ($row_has_location) {
        // Remaining row values go into the balloon
        if (!empty($this->rendered_fields[$i])) {
          $location->balloonText = implode('<br/>', $this->rendered_fields[$i]);
        }
        $locations[] = $location;
      }
    }
    $map_options = empty($this->options['ip_geoloc_map_of_view_options']) ? IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS : $this->options['ip_geoloc_map_of_view_options'];
    $map_style = empty($this->options['ip_geoloc_map_of_view_div_style']) ? IP_GEOLOC_MAP_DIV_DEFAULT_STYLE : $this->options['ip_geoloc_map_of_view_div_style'];
    $center_option = !isset($this->options['ip_geoloc_views_plugin_center_option']) ? IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION : $this->options['ip_geoloc_views_plugin_center_option'];

    if ($center_option >= IP_GEOLOC_MAP_CENTER_ON_VISITOR) {
      // Perform database IP lookup as backup/replacement for true location
      $resample = FALSE; // use existing location lat/long if these exist
      $store = TRUE; // store lat/long, if this is a new location
      $reverse_geocode = FALSE; // we only need lat/long, not the address
      $location = ip_geoloc_get_location_by_ip(ip_address(), $resample, $store, $reverse_geocode);
      $center_latlng = array($location['latitude'], $location['longitude']);
    }

    $output = theme(array('ip_geoloc_map'), array(
      'view' => $this->view,
      'locations' => $locations,
      'div_id' => 'ip-geoloc-map-of-view',
      'map_options' => $map_options,
      'map_style' => $map_style,
      'center_option' => $center_option,
      'center_latlng' => $center_latlng,
    ));
    return $output;
  }

  /**
   * Perform token replacement, convert timestamps to date strings etc. for
   * the entire result set. Store the rendered rows on the object.
   *
   * @param $result
   *   The result array from $this->view->result
   */
  function render_fields($result) {
    if (!$this->uses_fields()) {
      return;
    }
    if (!isset($this->rendered_fields)) {
      $this->rendered_fields = array();
      $field_ids = array_keys($this->view->field);
      foreach ($result as $i => $row) {
        $this->view->row_index = $i; // God knows why we need this...
        foreach ($field_ids as $field_id) {
          // add the field label if it's provided
          $label = $this->view->field[$field_id]->label();
          $element = '';
          $close_element = '';
          if ($label) {
            $label_type = $this->view->field[$field_id]->options['element_label_type'];
            if ($label_type) {
              $label_type = check_plain($label_type);
              $element = '<' . $label_type;
              $label_class = $this->view->field[$field_id]->options['element_label_class'];
              if ($label_class) {
                $element .= ' class="' . check_plain($label_class) . '"';
              }
              $element .= '>';
              $close_element = '</' . $label_type . '>';
            }
            if ($this->view->field[$field_id]->options['element_label_colon']) {
              $label .= ': ';
            }
            $this->rendered_fields[$i][$field_id] = $element . $label . $close_element . ' ' . $this->view->field[$field_id]->theme($row);
          } // otherwise render with no label
          else {
            $this->rendered_fields[$i][$field_id] = $this->view->field[$field_id]->theme($row);
          }
        }
        //$this->row_tokens[$i] = $this->view->field[$field_id]->get_render_tokens(array());
      }
      unset($this->view->row_index);
    }
    return $this->rendered_fields;
  }

}
