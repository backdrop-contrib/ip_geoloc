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

define('IP_GEOLOC_MAP_CENTER_FIXED', 0);
define('IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION', 1);
define('IP_GEOLOC_MAP_CENTER_ON_VISITOR', 2);

/**
 * Extract an array of locations from the supplied views_plugin_style.
 *
 * @param type $views_plugin_style
 * @return array of location objects, each containing lat/long and balloonText
 */
function ip_geoloc_plugin_style_extract_locations($views_plugin_style) {

  $latitude  = trim($views_plugin_style->options['ip_geoloc_views_plugin_latitude' ]);
  $longitude = trim($views_plugin_style->options['ip_geoloc_views_plugin_longitude']);

  if (empty($longitude)) {
    $longitude = $latitude; // when lat/long is a single field
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
 * @param $views_plugin_style
 * @param $result
 *   The result array on the view, e.g.
 */
function ip_geoloc_plugin_style_render_fields($views_plugin_style) {
  if (!$views_plugin_style->uses_fields()) {
    return;
  }
  if (!isset($views_plugin_style->rendered_fields)) {
    $views_plugin_style->rendered_fields = array();
    $field_ids = array_keys($views_plugin_style->view->field);
    foreach ($views_plugin_style->view->result as $i => $row) {
      $views_plugin_style->view->row_index = $i; // God knows why we need this...
      foreach ($field_ids as $field_id) {
        if ($views_plugin_style->view->field[$field_id]->options['exclude']) {
          continue;
        }
        // Add the field label if it's provided
        $label = $views_plugin_style->view->field[$field_id]->label();
        $element = '';
        $close_element = '';
        if ($label) {
          $label_type = $views_plugin_style->view->field[$field_id]->options['element_label_type'];
          if ($label_type) {
            $label_type = check_plain($label_type);
            $element = '<' . $label_type;
            $label_class = $views_plugin_style->view->field[$field_id]->options['element_label_class'];
            if ($label_class) {
              $element .= ' class="' . check_plain($label_class) . '"';
            }
            $element .= '>';
            $close_element = '</' . $label_type . '>';
          }
          if ($views_plugin_style->view->field[$field_id]->options['element_label_colon']) {
            $label .= ': ';
          }
          $views_plugin_style->rendered_fields[$i][$field_id] =
            $element . $label . $close_element . ' ' . $views_plugin_style->view->field[$field_id]->theme($row);
        } // otherwise render with no label
        else {
          $views_plugin_style->rendered_fields[$i][$field_id] = $views_plugin_style->view->field[$field_id]->theme($row);
        }
      }
      //$views_plugin_style->row_tokens[$i] = $views_plugin_style->view->field[$field_id]->get_render_tokens(array());
    }
    unset($views_plugin_style->view->row_index);
  }
  return $views_plugin_style->rendered_fields;
}