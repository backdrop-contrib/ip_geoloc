<?php

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
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['ip_geoloc_map_of_view_div_style'] = array(
      '#title' => t('Map style (CSS attributes)'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_map_of_view_div_style'],
      '#description' => t('If left blank, the default %default_style will result in a map of 300 pixels high, with a width bounded by the element that contains it. Separate style settings with semi-colons. Do not enter quotes or equal signs.',
        array('%default_style' => IP_GEOLOC_MAP_DIV_DEFAULT_STYLE))
    );
    $form['ip_geoloc_map_of_view_options'] = array(
      '#title' => t('Map options'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_map_of_view_options'],
      '#description' => t("If left blank, the default %default_options will produce a world map zoomed in to level 2. A list of map options can be found <a href='!google_map_docs'>here</a>. Remember to separate options with comma's, not semi-colons, and make sure your quotes match.", array(
        '%default_options' => IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS,
        '!google_map_docs' => DOC_GOOGLE_MAP_OPTIONS))
    );
    $form['ip_geoloc_views_plugin_latitude'] = array(
      '#title' => t('Name of latitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_latitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_latitude</strong>, unless you are viewing another module or a relationship, in which case you may have to prefix this name with the relevant table names, delimited by underscores.  If the latitude is stored in a <strong>text</strong> field, then enter the field\'s machine name here.')
    );
    $form['ip_geoloc_views_plugin_longitude'] = array(
      '#title' => t('Name of longitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_longitude'],
      '#description' => t('Use the default, <strong>ip_geoloc_longitude</strong>, unless you are viewing another module or a relationship, in which case you may have to prefix this name with the relevant table names, delimited by underscores.  If the longitude is stored in a <strong>text</strong> field, then enter the field\'s machine name here.')
    );
  }

  /**
   * Transform the View result in a list of maker locations and render as a map.
   *
   * @param type $result
   *   This is superfluous, same as $this->view->result.
   *
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
      $longitude = 'ip_geoloc_longitude';
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
      // First look for $latitude and $longitude as field values:
      if (!empty($row->{'field_' . $latitude}[0]['raw']['value']) && !empty($row->{'field_' . $longitude}[0]['raw']['value'])) {

        // If latitude and longitude are text fields, then their values will be
        // inside some arrays - $row->{latitude}[0]['raw']['value'].
        $location->latitude  = $row->{'field_' . $latitude}[0]['raw']['value'];
        $location->longitude = $row->{'field_' . $longitude}[0]['raw']['value'];
      }
      elseif (!empty($row->{$latitude}) && !empty($row->{$longitude})) {

        // Standard ip_geoloc fields: values are $row->{$latitude} and $row->{$longitude}
        $location->latitude  = $row->{$latitude};
        $location->longitude = $row->{$longitude};
      }
      else {
        $row_has_location = FALSE;
      }
      if ($row_has_location && !empty($this->rendered_fields)) {
        // Remaining row values go into the balloon
        $location->balloonText = implode('<br/>', $this->rendered_fields[$i]);
        $locations[] = $location;
      }
    }
    $map_options = $this->options['ip_geoloc_map_of_view_options'];
    $map_style = $this->options['ip_geoloc_map_of_view_div_style'];

    $output = theme(array('ip_geoloc_map'), array(
      'locations' => $locations,
      'div_id' => 'ip-geoloc-map-of-view',
      'map_options' => empty($map_options) ? IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS : $map_options,
      'map_style' => empty($map_style) ? IP_GEOLOC_MAP_DIV_DEFAULT_STYLE : $map_style,
      'view' => $this->view
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
