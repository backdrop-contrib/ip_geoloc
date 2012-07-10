<?php

/**
 * @file
 * ip_geoloc_plugin_style_map.inc
 *
 * Views Style plugin extension.
 */
require_once 'ip_geoloc_plugin_style.inc';

class ip_geoloc_plugin_style_map extends views_plugin_style {

  /**
   * Set default map options.
   */
  function option_definition() {
    $options = parent::option_definition();

    if (empty($latitude)) {
      $latitude  = module_exists('location') ? 'location_latitude' : 'ip_geoloc_latitude';
    }
    if (empty($longitude)) {
      $longitude = module_exists('location') ? 'location_longitude' : ($latitude == 'ip_geoloc_latitude' ? 'ip_geoloc_longitude' : $latitude); // for field-based modules
    }
    $options['ip_geoloc_map_of_view_div_style']  = array('default' => '');
    $options['ip_geoloc_map_of_view_options']    = array('default' => '');
    $options['ip_geoloc_views_plugin_latitude']  = array('default' => $latitude);
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => $longitude);
    $options['ip_geoloc_views_plugin_visitor_marker'] = array('default' => '<none>');
    $options['ip_geoloc_views_plugin_center_option'] = array('default' => IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION);
    $options['ip_geoloc_views_plugin_visitor_location_gps'] = array('default' => TRUE);
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
      '#description' => t('Use the default, <strong>ip_geoloc_latitude</strong>, unless your view involves other modules or relationships, in which case you may have to prefix the name with the relevant table name(s), delimited by underscores. For instance, if you use a view based on the <strong>Location</strong> module, use <strong>location_latitude</strong>. If the latitude is stored in a <strong>text field</strong>, then enter the field\'s machine name, e.g. when using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules enter <em>field_myname</em>. Valid field names are on <a href="/admin/reports/views-fields">this page</a>.')
    );
    $form['ip_geoloc_views_plugin_longitude'] = array(
      '#title' => t('Name of longitude field in Views query'),
      '#type' => 'textfield',
      '#size' => 127,
      '#default_value' => $this->options['ip_geoloc_views_plugin_longitude'],
      '#description' => t('See comments above. When using the <strong>Geofield</strong> or <strong>Geolocation field</strong> modules you can leave this field empty.')
    );
    $form['ip_geoloc_views_plugin_visitor_marker'] = array(
      '#title' => t('Visitor marker color'),
      '#type' => 'textfield',
      '#size' => 6,
      '#default_value' => $this->options['ip_geoloc_views_plugin_visitor_marker'],
      '#description' => t("Show visitor marker in the color specified. %color_code is bright green. Note that an additional web service call will be made to retrieve the marker image, unless this field is left blank, which will result in a standard red marker. Enter <em>&lt;none&gt;</em> if you do not wish to display the visitor's current location.",
        array('%color_code' => '00FF00'))
    );
    $form['ip_geoloc_views_plugin_center_option'] = array(
      '#title' => t('Map centering options'),
      '#type' => 'radios',
      '#default_value' => $this->options['ip_geoloc_views_plugin_center_option'],
      '#options' => array(
        IP_GEOLOC_MAP_CENTER_FIXED => t('Fixed center (see note below).'),
        IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION => t('Use the first location returned by the view as the center of the map.'),
        IP_GEOLOC_MAP_CENTER_ON_VISITOR => t("Center the map on the visitor's current location."),
      ),
      '#description' => t('If you choose the first option you may center the map via the special <strong>Map options</strong> <em>"centerLat"</em> and <em>"centerLng"</em> for example: %center_example', array(
        '%center_example' => '{ "mapTypeId":"roadmap", "centerLat":-37.8, "centerLng":145 }'))
    );
    $form['ip_geoloc_views_plugin_visitor_location_gps'] = array(
      '#title' => t('Use HTML5 style of visitor location retrieval'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['ip_geoloc_views_plugin_visitor_location_gps'],
      '#description' => t('This is subject to the visitor willing to share their location when prompted by their browser. If they decline, or if this box is not ticked, an IP address lookup will be attempted instead. In case the lookup fails, for instance because you have not enabled Smart IP or GeoIP, make sure you specify "centerLat" and "centerLng" in the <strong>Map options</strong> above to avoid an empty grey canvas.')
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

    ip_geoloc_plugin_style_render_fields($this);
    $locations = ip_geoloc_plugin_style_extract_locations($this);

    $map_options = empty($this->options['ip_geoloc_map_of_view_options']) ? IP_GEOLOC_RECENT_VISITORS_MAP_OPTIONS : $this->options['ip_geoloc_map_of_view_options'];
    $map_style = empty($this->options['ip_geoloc_map_of_view_div_style']) ? IP_GEOLOC_MAP_DIV_DEFAULT_STYLE : check_plain($this->options['ip_geoloc_map_of_view_div_style']);

    if (empty($this->options['ip_geoloc_views_plugin_visitor_marker'])) {
      $visitor_marker = TRUE; // default to standard red marker
    }
    else {
      $visitor_marker = trim($this->options['ip_geoloc_views_plugin_visitor_marker']);
      $visitor_marker = ($visitor_marker == '<none>') ? FALSE : check_plain($visitor_marker);
    }

    $center_option = !isset($this->options['ip_geoloc_views_plugin_center_option']) ? IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION : $this->options['ip_geoloc_views_plugin_center_option'];
    $center_latlng = FALSE;

    $visitor_location_gps = !empty($this->options['ip_geoloc_views_plugin_visitor_location_gps']);

    if ($visitor_marker || $center_option >= IP_GEOLOC_MAP_CENTER_ON_VISITOR) {
      // Perform database IP lookup as backup/replacement for true location
      $resample = FALSE; // use existing lat/long if these exist
      $store = TRUE; // store lat/long, if this is a new location
      $reverse_geocode = FALSE; // we only need lat/long, not the address
      $location = ip_geoloc_get_location_by_ip(ip_address(), $resample, $store, $reverse_geocode);
      if (isset($location['latitude']) && isset($location['longitude'])) {
        $center_latlng = array($location['latitude'], $location['longitude']);
      }
    }

    $output = theme(array('ip_geoloc_map'), array(
      'view' => $this->view,
      'locations' => $locations,
      'div_id' => 'ip-geoloc-map-of-view-' . $this->view->name,
      'map_options' => $map_options,
      'map_style' => $map_style,
      'visitor_marker' => $visitor_marker,
      'center_option' => $center_option,
      'center_latlng' => $center_latlng,
      'visitor_location_gps' => $visitor_location_gps
    ));
    return $output;
  }

}
