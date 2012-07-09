<?php
/**
 * @file
 * ip_geoloc_plugin_style.inc
 *
 * Helper functions for Views style plugins, in particular to extract location
 * data (lat/long columns) from the associated view.
 *
 * @todo turn this into a common base clase for ip_geoloc_style_*.inc.
 */

/**
 * Extract an array of locations from the supplied views_plugin_style.
 *
 * @param type $views_plugin_style
 * @return array of location objects, each containing lat/long and balloonText
 */
function ip_geoloc_plugin_style_extract_locations($views_plugin_style) {

  $latitude  = trim($views_plugin_style->options['ip_geoloc_views_plugin_latitude' ]);
  $longitude = trim($views_plugin_style->options['ip_geoloc_views_plugin_longitude']);

  if (empty($latitude)) {
    $latitude = 'ip_geoloc_latitude';
  }
  if (empty($longitude)) {
    $longitude = ($latitude == 'ip_geoloc_latitude') ? 'ip_geoloc_longitude' : $latitude; // for field-based modules
  }

  $locations = array();
  foreach ($views_plugin_style->view->result as $i => $row) {
    $location = new stdClass();
    $row_has_location = TRUE;
    // First look for $latitude and $longitude as node field values:
    if (!empty($row->{'field_' . $latitude}[0]['raw']) && !empty($row->{'field_' . $longitude}[0]['raw'])) {

      // When based on Geofield or Geolocation Field modules lat/long are as follows
      // Geofield   : field_<field_name>[0]['raw']['lat'] and field_<field_name>[0]['raw']['lon']
      // Geolocation: field_<field_name>[0]['raw']['lat'] and field_<field_name>[0]['raw']['lng']
      if (isset($row->{'field_' . $latitude}[0]['raw']['lon'])) {
        $field_name = $latitude;
        $location->latitude  = $row->{'field_' . $field_name}[0]['raw']['lat'];
        $location->longitude = $row->{'field_' . $field_name}[0]['raw']['lon'];
      }
      elseif (isset($row->{'field_' . $latitude}[0]['raw']['lng'])) {
        $field_name = $latitude;
        $location->latitude  = $row->{'field_' . $field_name}[0]['raw']['lat'];
        $location->longitude = $row->{'field_' . $field_name}[0]['raw']['lng'];
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
      if (!empty($views_plugin_style->rendered_fields[$i])) {
        $location->balloonText = implode('<br/>', $views_plugin_style->rendered_fields[$i]);
      }
      $locations[] = $location;
    }
  }
  return $locations;
}


/**
 * Perform token replacement, convert timestamps to date strings etc. for
 *
 * Store the rendered rows on the object passed in, which will typically be an
 * instance of class views_plugin_style or subclass.
 * Note that fields that have their Exclude box ticked, won't be rendered,
 * Typical candidates for exclusion are the latitude and longitude fields.
 *
 * @param $view_plugin_style
 * @param $result
 *   The result array on the view, e.g.
 */
function ip_geoloc_plugin_style_render_fields($view_plugin_style) {
  if (!$view_plugin_style->uses_fields()) {
    return;
  }
  if (!isset($view_plugin_style->rendered_fields)) {
    $view_plugin_style->rendered_fields = array();
    $field_ids = array_keys($view_plugin_style->view->field);
    foreach ($view_plugin_style->view->result as $i => $row) {
      $view_plugin_style->view->row_index = $i; // God knows why we need this...
      foreach ($field_ids as $field_id) {
        if ($view_plugin_style->view->field[$field_id]->options['exclude']) {
          continue;
        }
        // Add the field label if it's provided
        $label = $view_plugin_style->view->field[$field_id]->label();
        $element = '';
        $close_element = '';
        if ($label) {
          $label_type = $view_plugin_style->view->field[$field_id]->options['element_label_type'];
          if ($label_type) {
            $label_type = check_plain($label_type);
            $element = '<' . $label_type;
            $label_class = $view_plugin_style->view->field[$field_id]->options['element_label_class'];
            if ($label_class) {
              $element .= ' class="' . check_plain($label_class) . '"';
            }
            $element .= '>';
            $close_element = '</' . $label_type . '>';
          }
          if ($view_plugin_style->view->field[$field_id]->options['element_label_colon']) {
            $label .= ': ';
          }
          $view_plugin_style->rendered_fields[$i][$field_id] =
            $element . $label . $close_element . ' ' . $view_plugin_style->view->field[$field_id]->theme($row);
        } // otherwise render with no label
        else {
          $view_plugin_style->rendered_fields[$i][$field_id] = $view_plugin_style->view->field[$field_id]->theme($row);
        }
      }
      //$view_plugin_style->row_tokens[$i] = $view_plugin_style->view->field[$field_id]->get_render_tokens(array());
    }
    unset($view_plugin_style->view->row_index);
  }
  return $view_plugin_style->rendered_fields;
}