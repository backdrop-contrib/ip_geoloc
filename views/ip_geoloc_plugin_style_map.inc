<?php

class ip_geoloc_plugin_style_map extends views_plugin_style {

  /**
   * Set default map options.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['ip_geoloc_map_of_view_div_style'] = array(
      'default' => variable_get('ip_geoloc_map_of_view_div_style', IP_GEOLOC_MAP_DIV_DEFAULT_STYLE));
    $options['ip_geoloc_map_of_view_options'] = array(
      'default' => variable_get('ip_geoloc_map_of_view_options', IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS));
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['ip_geoloc_map_of_view_div_style'] = array(
      '#title' => t('Map style (CSS attributes)'),
      '#type' => 'textfield',
      '#size' => 150,
      '#default_value' => $this->options['ip_geoloc_map_of_view_div_style'],
      '#description' => t('The default %default_style results in a map of 300 pixels high, with a width bounded by the element that contains it. Separate options with semi-colons. Do not enter quotes or equal signs.',
        array('%default_style' => IP_GEOLOC_MAP_DIV_DEFAULT_STYLE))
    );
    $form['ip_geoloc_map_of_view_options'] = array(
      '#title' => t('Map options'),
      '#type' => 'textfield',
      '#size' => 150,
      '#default_value' => $this->options['ip_geoloc_map_of_view_options'],
      '#description' => t("The default %default_options produces a world map zoomed in to level 2. A list of map options can be found <a href='!google_map_docs'>here</a>. Remember to separate options with comma's, not semi-colons, and make sure your quotes match.", array(
        '%default_options' => IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS,
        '!google_map_docs' => DOC_GOOGLE_MAP_OPTIONS))
    );
  }

  function render($result) {

    if (!empty($this->view->live_preview)) {
      return t('The map format is incompatible with the preview function.');
    }
    $locations = array();
    foreach ($this->view->result as $row) {
      if (!empty($row->ip_geoloc_latitude) && !empty($row->ip_geoloc_longitude)) {
        $location = new stdClass();
        $location->latitude  = $row->ip_geoloc_latitude;
        $location->longitude = $row->ip_geoloc_longitude;
        unset($row->ip_geoloc_latitude);
        unset($row->ip_geoloc_longitude);
        // Remaining values on the row go into balloon text.
        $location->balloonText = implode('<br/>', array_values((array)$row));
        $locations[] = $location;
      }
    }
    $map_options = variable_get('ip_geoloc_map_of_view_options', IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS);
    $map_style = variable_get('ip_geoloc_map_of_view_div_style', IP_GEOLOC_MAP_DIV_DEFAULT_STYLE);
    $output = theme(array('ip_geoloc_map'), array(
      'locations' => $locations,
      'div_id' => 'ip-geoloc-map-of-view',
      'map_options' => $map_options,
      'map_style' => $map_style,
      'view' => clone($this->view)
    ));
    return $output;
  }

}
