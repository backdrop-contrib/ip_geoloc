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

  if (empty($views_plugin_style->options['ip_geoloc_views_plugin_differentiator_field'])) {
    $differentiator_color_mappings = array();
  }
  else {
    $differentiator_name = $views_plugin_style->options['ip_geoloc_views_plugin_differentiator_field'];
    $differentiator_color_mappings = variable_get('ip_geoloc_' . $differentiator_name . '_color_mappings', array());
  }
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

    if (!empty($differentiator_name)) {
      if (!empty($row->{$differentiator_name})) {
        $differentiator_value = $row->{$differentiator_name};
      }
      elseif (!empty($row->{'field_' . $differentiator_name})) {
        $differentiator_value = $row->{'field_' . $differentiator_name};
        if (is_array($differentiator_value)) {
          if (isset($differentiator_value[0]['raw']['tid'])) {
            $differentiator_value = $differentiator_value[0]['raw']['tid'];
          }
          elseif (isset($differentiator_value[0]['raw']['value'])) {
            $differentiator_value = $differentiator_value[0]['raw']['value'];
          }
          elseif (isset($differentiator_value[0]['rendered']['#markup'])) {
            $differentiator_value = $differentiator_value[0]['rendered']['#markup'];
          }
        }
      }
      if (!empty($differentiator_color_mappings[$differentiator_value])) {
        $location->marker_color = $differentiator_color_mappings[$differentiator_value];
      }
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

/**
 *  Menu callback, see ip_geoloc_menu().
 */
function ip_geoloc_differentiator_to_color_mappings($form, &$form_state) {

  if (empty($form_state['differentiator'])) {
    $form_state['differentiator'] = arg(1);
    $form_state['is_openlayers'] = (arg(2) == 'ol');
  }
  $differentiator = $form_state['differentiator'];
  $is_openlayers = $form_state['is_openlayers'];
//$field = field_info_field($differentiator);
  $instance = ip_geoloc_get_field_instance($differentiator);
  $differentiator_label = isset($instance) ? $instance['label'] : $differentiator;

  $form['description'] = array(
    '#markup' => '<div>' .
      t('Use this page to associate values of the %differentiator differentiator from your view with corresponding location marker colors on the map.', array(
        '%differentiator' => $differentiator_label)) .
    '</div>',
  );

  // Add a wrapper for the table and buttons.
  $form['field-mappings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Translate %differentiator values on the left to marker colors on the right', array(
      '%differentiator' => $differentiator_label
    )),
    '#prefix' => '<div id="field-mapppings-wrapper" class="clearfix">',
    '#suffix' => '</div>',
    '#description' => t('Omitted %differentiator values will result in the default marker being shown for those locations.', array(
      '%differentiator' => $differentiator_label
    ))
  );

  // Container for the table, its id is used in the AJAX callback
  $form['field-mappings']['table'] = array(
    '#theme' => 'ip_geoloc_field_mapping_table',
    '#prefix' => '<div id="field-mapping-table">',
    '#suffix' => '</div>',
  );

  $field_mappings = variable_get('ip_geoloc_' . $differentiator . '_color_mappings', array('' => 'blue', '' => 'red'));
  // First the saved rows (or default rows if none saved)...
  // @todo: if $field['cardinality'] > 1, compress multiple differentiator values
  // for the same color together in a single row
  $row = 0;
  foreach ($field_mappings as $differentiator_value => $color) {
    if (isset($form_state['num_mappings']) && $row >= $form_state['num_mappings']) {
      break;
    }
    $form['field-mappings']['table'][$row] = _ip_geoloc_table_row_form($is_openlayers, $row, $differentiator, $differentiator_value, $color);
    $row++;
  }
  // ... then the not yet saved, empty rows
  if (!isset($form_state['num_mappings'])) {
    $form_state['num_mappings'] = count($field_mappings);
  }
  while ($row < $form_state['num_mappings']) {
    $form['field-mappings']['table'][$row] = _ip_geoloc_table_row_form($is_open_layers, $row, $differentiator);
    $row++;
  }

  $form['field-mappings']['add-another'] = array(
    '#type' => 'submit',
    '#value' => ($form_state['num_mappings'] == 0) ? t('Add an association') : t('Add another association'),
    '#weight' => 1,
    '#submit' => array('ip_geoloc_add_another_mapping_submit'),
    '#ajax' => array(
      'callback' => 'ip_geoloc_add_another_mapping_js',
      'wrapper' => 'field-mapping-table',
      'effect' => 'slide'
    )
  );
  if ($form_state['num_mappings'] > 0) {
    $form['field-mappings']['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove bottom association'),
      '#weight' => 2,
      '#submit' => array('ip_geoloc_remove_bottom_mapping_submit'),
      '#ajax' => array(
        'callback' => 'ip_geoloc_add_another_mapping_js',
        'wrapper' => 'field-mapping-table',
        'effect' => 'fade'
      )
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save associations'),
  );
  return $form;
}

/**
 * Submit handler for the "Add another association" button.
 *
 * Increments the counter and forces a form rebuild.
 */
function ip_geoloc_add_another_mapping_submit($form, &$form_state) {
  $form_state['num_mappings']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Remove" button.
 *
 * Decrements the counter and formces a form rebuild.
 */
function ip_geoloc_remove_bottom_mapping_submit($form, &$form_state) {
  if ($form_state['num_mappings'] > 0) {
    $form_state['num_mappings']--;
  };
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback in response to new mapping rows being added to the form.
 */
function ip_geoloc_add_another_mapping_js($form, $form_state) {
  if ($form_state['num_mappings'] == 0) {
    unset($form_state['values']['remove']);
  }
  return $form['field-mappings']['table'];
}

/**
 * ip_geoloc_differentiator_to_color_mappings submit handler.
 */
function ip_geoloc_differentiator_to_color_mappings_submit($form, &$form_state) {
  $differentiator = $form_state['differentiator'];
  $field_mappings = array();
  if (!empty($form_state['values']['table'])) {
    foreach ($form_state['values']['table'] as $mapping) {
      if (is_array($mapping[$differentiator])) { // e.g mapping['field_file_under'][0]['tid']
        foreach ($mapping[$differentiator][LANGUAGE_NONE] as $key => $value_array) {
          $differentiator_value = reset($value_array);
          if (!empty($differentiator_value)) {
            $field_mappings[$differentiator_value] = $mapping['color'];
          }
        }
      }
      else { // text
        $differentiator_value = trim($mapping[$differentiator]);
        if (!empty($differentiator_value)) {
          $field_mappings[$differentiator_value] = $mapping['color'];
        }
      }
    }
  }
  variable_set('ip_geoloc_' . $differentiator . '_color_mappings', $field_mappings);
}

function _ip_geoloc_table_row_form($is_openlayers, $row, $differentiator, $differentiator_value = '', $color = '') {

  if (drupal_substr($differentiator, 0, 6) == 'field_') {
    $field = field_info_field($differentiator);
    $instance = ip_geoloc_get_field_instance($differentiator);
    $instance['label'] = ''; // no label, unless other modules override this
    $items[0][key($field['columns'])] = $differentiator_value;
    $form['#parents'] = array();
    $form_state = array();
    $form = field_default_form($instance['entity_type'], NULL, $field, $instance, LANGUAGE_NONE, $items, $form, $form_state);
  }
  else {
    $form[$differentiator] = array(
      '#type' => 'textfield',
      '#default_value' => $differentiator_value
    );
  }
  if ($is_openlayers) {
    $num_location_marker_layers = variable_get('ip_geoloc_num_location_marker_layers', IP_GEOLOC_DEF_NUM_MARKER_LAYERS);
    $marker_layers = array();
    for ($layer = 1; $layer <= $num_location_marker_layers; $layer++) {
      $marker_layers[$layer] = t('Marker layer') . " #$layer";
    }
  }
  $form['color'] = array(
    '#type' => 'select',
    '#default_value' => $color,
    '#options' => $is_openlayers ? $marker_layers : ip_geoloc_marker_colors()
  );

  // We'll manually set the #parents property of these fields so that their
  // values appear in the $form_state['values']['table'] array.
  $form[$differentiator]['#parents'] = array('table', $row, $differentiator);
  $form['color']['#parents'] = array('table', $row, 'color');

  return $form;
}

/**
 * Return HTML for differentiator to color mapping table.
 *
 * @param $variables
 *   An associative array containing $variables['form']: a render element
 *   representing the form.
 *
 * @ingroup themeable
 */
function theme_ip_geoloc_field_mapping_table($variables) {
  // Use the first form child to find out the name of the differentiator.
  $form = $variables['form'];
  $form_children = element_children($form);
  $key = reset($form_children);
  foreach ($form[$key] as $attribute_name => $element) {
    if (drupal_substr($attribute_name, 0, 1) != '#' && $attribute_name != 'color') {
      $differentiator = $attribute_name;
      break;
    }
  }
  if (empty($differentiator)) {
    return '';
  }
  $instance = ip_geoloc_get_field_instance($differentiator);
  $differentiator_label = isset($instance) ? $instance['label'] : $differentiator;

  $headers = array(
    t('%differentiator value', array('%differentiator' => $differentiator_label)),
    t('Corresponding location marker color')
  );
  $rows = array();
  foreach (element_children($form) as $key) {
    $row = array('data' => array(), 'class' => array());
    $row['data'][] = drupal_render($form[$key][$differentiator]);
    $row['data'][] = drupal_render($form[$key]['color']);
    $rows[] = $row;
  }

  $output = theme('table', array('header' => $headers, 'rows' => $rows, 'attributes' => array('id' => 'differentiator-mapping-table')));
  $output .= drupal_render_children($form);
  return $output;
}

function ip_geoloc_get_field_instance($field_name) {
  foreach (field_info_instances() as $type_bundles) {
    foreach ($type_bundles as $bundle_instances) {
      foreach ($bundle_instances as $f_name => $instance) {
        if ($f_name == $field_name) {
          return $instance;
        }
      }
    }
  }
  return NULL;
}