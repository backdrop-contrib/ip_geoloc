<?php

/**
 * @file
 * ip_geoloc_plugin_style_leaflet.inc
 *
 * Views Style plugin extension for Leaflet (if enabled).
 */

require_once 'ip_geoloc_plugin_style.inc';

define('LEAFLET_MARKERCLUSTER_EXCLUDE_FROM_CLUSTER', 0x01);

class ip_geoloc_plugin_style_leaflet extends views_plugin_style {

  /**
   * Set default Leaflet options.
   */
  public function option_definition() {
    $options = parent::option_definition();

    // The leaflet.module default.
    $options['map'] = array('default' => 'OSM Mapnik');
    $options['map_height'] = array('default' => 300);

    $latitude  = module_exists('location') ? 'location_latitude' : 'ip_geoloc_latitude';
    $longitude = module_exists('location') ? 'location_longitude' : ($latitude == 'ip_geoloc_latitude' ? 'ip_geoloc_longitude' : $latitude);
    $options['ip_geoloc_views_plugin_latitude'] = array('default' => $latitude);
    $options['ip_geoloc_views_plugin_longitude'] = array('default' => $longitude);

    $options['default_marker'] = array(
      'contains' => array(
        'default_marker_color' => array('default' => ''),
        'default_marker_special_char' => array('default' => ''),
        'default_marker_special_char_class' => array('default' => ''),
      ),
    );
    $options['visitor_marker'] = array(
      'contains' => array(
        'visitor_marker_color' => array('default' => ''),
        'visitor_marker_special_char' => array('default' => ''),
        'visitor_marker_special_char_class' => array('default' => ''),
      ),
    );

    $options['differentiator'] = array(
      'contains' => array(
        'differentiator_field' => array('default' => ''),
      ),
    );

    $options['center_option'] = array('default' => 0);

    $options['tooltips'] = array(
      'contains' => array(
        'marker_tooltip' => array('default' => ''),
      ),
    );
    $options['tags'] = array(
      'contains' => array(
        'marker_tag' => array('default' => ''),
        'tag_css_class' => array('default' => 'tag-inside-marker'),
      ),
    );

    $options['cluster_radius'] = array('default' => module_exists('leaflet_markercluster') ? 80 : '');
    $options['cluster_differentiator'] = array(
      'contains' => array(
        'cluster_differentiator_field' => array('default' => ''),
        'zoom_ranges' => array('default' => array())
      ),
    );

    $options['empty_map_center'] = array('default' => '');

    $options['map_options'] = array(
      'contains' => array(
        'maxzoom' => array('default' => 18),
        'zoom' => array('default' => 2),
        'scrollwheelzoom' => array('default' => TRUE),
        'dragging' => array('default' => TRUE),
      ),
    );
    return $options;
  }

  /**
   * Implements options_form().
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $path = drupal_get_path('module', 'ip_geoloc');
    $select_css_file = strpos(variable_get('ip_geoloc_marker_directory'), 'amarkers') ? 'ip_geoloc_admin_select_amarkers.css' : 'ip_geoloc_admin_select_markers.css';

    $form['#attached'] = array('css' => array(
      "$path/css/ip_geoloc_admin.css",
      "$path/css/$select_css_file",
    ));
    $maps = array();
    foreach (ip_geoloc_plugin_style_leaflet_map_get_info() as $key => $map) {
      $maps[$key] = $map['label'];
    }
    $form['map'] = array(
      '#title' => t('Map'),
      '#type' => 'select',
      '#options' => $maps,
      '#default_value' => $this->options['map'],
      '#required' => TRUE,
      '#weight' => -2,
    );
    $form['map_height'] = array(
      '#title' => t('Map height'),
      '#type' => 'textfield',
      '#field_suffix' => t('px'),
      '#size' => 4,
      '#default_value' => $this->options['map_height'],
      '#description' => t('The default produces a map of 300 pixels high with a width extending to its bounding container.'),
      '#weight' => -1,
    );

    $form_state['renderer'] = 'leaflet';
    ip_geoloc_plugin_style_bulk_of_form($this, $form, $form_state);
    unset($form['default_marker_color']);

    $desc1 = t('In addition to selecting a color, you may superimpose a special icon on top of each marker. <br/><a target="fsymbols" href="!url_fsymbols">fsymbols</a> characters can be copied and pasted straight into the <strong>Font icon character</strong> field. <br/>Other libraries like <a target="fontawesome" href="!url_fontawesome">Font Awesome</a> and <a target="flaticon" href="!url_flaticon">flaticon</a> use names that you type in the <strong>Font icon class</strong> field.', array(
      '!url_fsymbols' => url('http://fsymbols.com'),
      '!url_fontawesome' => url('http://fortawesome.github.io/Font-Awesome/cheatsheet'),
      '!url_flaticon' => url('http://flaticon.com'),
    ));
    $desc2 = t('<em>fsymbols</em> require no further installation. For other libraries see the <a target="readme" href="!url_readme">README</a>.', array(
      '!url_readme' => url("$path/README.txt"),
    ));

    $desc3 = t('All this works best with the markers from the <em>/amarkers</em> directory, configurable <a target="config" href="!url_config">here</a>.', array(
      '!url_config' => url('admin/config/system/ip_geoloc'),
    ));
    $form['default_marker'] = array(
      '#type' => 'fieldset',
      '#title' => t('Default marker style'),
      '#description' => $desc1 . '<br/>' . $desc2 . '<br/>' . $desc3,
      '#weight' => 25,
    );
    $form['default_marker']['default_marker_color'] = array(
      '#title' => t('Style/color'),
      '#type' => 'select',
      '#default_value' => $this->options['default_marker']['default_marker_color'],
      '#options' => ip_geoloc_marker_colors(),
      '#description' => t('Select an image to use for all location markers whose images are not overridden by the <strong>Location differentiator</strong> below.'),
      '#attributes' => array('class' => array('marker-color')),
    );
    $form['default_marker']['default_marker_special_char'] = array(
      '#title' => t('Font icon character'),
      '#type' => 'textfield',
      '#size' => 8,
      '#default_value' => $this->options['default_marker']['default_marker_special_char'],
      '#description' => t('Paste directly from <a target="fsymbols" href="!url_fsymbols">fsymbols</a>.', array('!url_fsymbols' => url('http://fsymbols.com'))),
    );
    $desc4 =  t('Use the class name from the font icon library you are using. Append <strong>light</strong>, <strong>dark</strong> or <strong>red</strong> to change the color. Examples:<br/>Font Awesome: <strong>fa fa-beer light</strong><br/>flaticon: <strong>flaticon-bicycle12 red</strong>');
    $form['default_marker']['default_marker_special_char_class'] = array(
      '#title' => t('Font icon class'),
      '#type' => 'textfield',
      '#size' => 25,
      '#default_value' => $this->options['default_marker']['default_marker_special_char_class'],
      '#description' => $desc4,
    );

    $visitor_marker_colors = array(
      'none' => '<' . t('none') . '>') +
      ip_geoloc_marker_colors();
    unset($visitor_marker_colors['0']);

    $form['visitor_marker'] = array(
      '#type' => 'fieldset',
      '#title' => t('Visitor marker style'),
      '#weight' => 27,
    );
    $form['visitor_marker']['visitor_marker_color'] = array(
      '#title' => t('Style/color'),
      '#type' => 'select',
      '#multiple' => FALSE,
      '#default_value' => $this->options['visitor_marker']['visitor_marker_color'],
      '#options' => $visitor_marker_colors,
      '#attributes' => array('class' => array('marker-color')),
    );
    $form['visitor_marker']['visitor_marker_special_char'] = array(
      '#title' => t('Font icon character'),
      '#type' => 'textfield',
      '#size' => 8,
      '#default_value' => $this->options['visitor_marker']['visitor_marker_special_char'],
      '#description' => t('Paste directly from <a target="fsymbols" href="!url_fsymbols">fsymbols</a>.', array('!url_fsymbols' => url('http://fsymbols.com'))),
    );
    $form['visitor_marker']['visitor_marker_special_char_class'] = array(
      '#title' => t('Font icon class'),
      '#type' => 'textfield',
      '#size' => 25,
      '#default_value' => $this->options['visitor_marker']['visitor_marker_special_char_class'],
      '#description' => t('As above.'),
    );

    $form['center_option']['#options'][0] = t('Auto-box to fit all markers (include visitor marker if color <strong>not</strong> set to &lt;none&gt;)');

    $fields = ip_geoloc_get_display_fields($this->display->handler, FALSE);
    $form['tooltips'] = array(
      '#title' => t('Marker tooltips'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => empty($this->options['tooltips']['marker_tooltip']),
      '#description' => t('In addition to its balloon each marker may have a tooltip. A tooltip is a short text that appears when visitors hover their mouse over a marker.'),
      '#weight' => 60,
    );
    $form['tooltips']['marker_tooltip'] = array(
      '#title' => t('Views field to populate tooltips'),
      '#type' => 'select',
      '#default_value' => $this->options['tooltips']['marker_tooltip'],
      '#options' => $fields,
      '#description' => t('Example: "Content: Title"'),
    );

    $form['tags'] = array(
      '#title' => t('Marker tags'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => empty($this->options['tags']['marker_tag']),
      '#description' => t('Each marker may have a tag. A tag is a number or short text shown permanently above, below or inside the marker.'),
      '#weight' => 70,
    );
    $form['tags']['marker_tag'] = array(
      '#title' => t('Views field to populate tags'),
      '#type' => 'select',
      '#default_value' => $this->options['tags']['marker_tag'],
      '#options' => $fields,
      '#description' => t('Example: "Content: Title". Use "Global: View result counter" if you want to number your locations.'),
    );
    $form['tags']['tag_css_class'] = array(
      '#title' => t('Tag position and style'),
      '#type' => 'textfield',
      '#default_value' => $this->options['tags']['tag_css_class'],
      '#description' => t('The CSS class or classes applied to each tag. Tagged marker CSS classes coming with this module are <strong>tag-above-marker</strong>, <strong>tag-below-marker</strong> and <strong>tag-inside-marker</strong>. If you opted to have <em>no markers</em>, i.e. tags only, you may use <strong>tag-rounded-corners</strong> or <strong>tag-pointy-circle</strong>, which is recommended for numbers. You may also create your own CSS classes and use them here.'),
    );
    $selected_map = ip_geoloc_plugin_style_leaflet_map_get_info($this->options['map']);
    $zoom_top = 18;
    if (isset($selected_map['settings']['maxZoom'])) {
      $zoom_top = $selected_map['settings']['maxZoom'];
    }
    $form['map_options'] = array(
      '#title' => t('More map options'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      // Could be empty($this->options['map_options']['zoom']), ?
      '#collapsed' => TRUE,
      '#weight' => 200,
    );
    $form['map_options']['maxzoom'] = array(
      '#title' => t('Maximum zoom level (0..@zoomtop)', array('@zoomtop' => $zoom_top)),
      '#type' => 'textfield',
      '#size' => 2,
      '#default_value' => $this->options['map_options']['maxzoom'],
      '#description' => t('Note that not all maps support all zoom levels.'),
    );
    $initial_zoom_max = $zoom_top;
    if (is_numeric($this->options['map_options']['maxzoom'])) {
      $initial_zoom_max = min($zoom_top, $this->options['map_options']['maxzoom']);
    }
    $form['map_options']['zoom'] = array(
      '#title' => t('Initial zoom level (0..@maxzoom)', array('@maxzoom' => $initial_zoom_max)),
      '#type' => 'textfield',
      '#size' => 2,
      '#default_value' => $this->options['map_options']['zoom'],
      '#description' => t('Does not apply to auto-box centering except when only one or no markers are shown.'),
    );
    $form['map_options']['scrollwheelzoom'] = array(
      '#title' => t('Enable scroll wheel zoom'),
      '#type' => 'select',
      '#default_value' => $this->options['map_options']['scrollwheelzoom'],
      '#options' => array(TRUE => t('Yes'), FALSE => t('No')),
    );
    $form['map_options']['dragging'] = array(
      '#title' => t('Dragging/Panning of the map'),
      '#type' => 'select',
      '#default_value' => $this->options['map_options']['dragging'],
      '#options' => array(TRUE => t('Yes'), FALSE => t('No')),
    );

    $form['cluster_radius'] = array(
      '#title' => t('Marker cluster radius'),
      '#type' => 'textfield',
      '#field_suffix' => t('px'),
      '#size' => 4,
      '#default_value' => $this->options['cluster_radius'],
      '#description' => module_exists('leaflet_markercluster') ? t('Leave empty to disable clustering.') : t('Requires the <a href="!url">Leaflet MarkerCluster</a> module and corresponding javascript library.', array(
        '!url' => url('http://drupal.org/project/leaflet_markercluster'),
      )),
      '#weight' => 100,
    );

    // Add wrapper for cluster differentiator drop-down and region level
    // specifiers.
    // The id in the prefix must match the AJAX submit handlers below.
    $warning = '<p><strong>' . t('This feature is under development. Not ready, yet.') . '</strong></p>';

    $form['cluster_differentiator'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Marker cluster differentiator'),
      '#description' => $warning . t('While the location differentiator determines the color of individual markers, the cluster differentiator decides which <em>geographical region</em> a marker belongs at available zoom-levels. Examples of regions are the hierarchy of country, state, suburb or areas like wine regions, breeding grounds etc. At any zoom level markers belonging to different regions will not merge together, no matter how close they are on the map. This prevents markers on opposite sides of a region border from being included in the same cluster. <br/>Make sure the cluster differentiator you wish to use is included in this view as a field. Fields that are particularly suitable for clustering are AddressFields and taxonomies with depth. You may tick the "Excluded" box on the field, if you do not wish it to appear in the marker balloons. Markers that do not belong to any cluster group are deemed to be members of the <em>global</em> group. The exception is the visitor marker, which is excluded from clustering.'),
      '#prefix' => '<div id="cluster-differentiator-wrapper">',
      '#suffix' => '</div>',
      '#weight' => 103,
    );
    $cluster_differentiator = $this->options['cluster_differentiator']['cluster_differentiator_field'];
    if (isset($form_state['triggering_element'])) {
      // Get here when any form element with #ajax was changed/clicked causing
      // an auto-rebuild of the form.
      if (strpos($form_state['triggering_element']['#id'], 'cluster-differentiator-field') > 0) {
        // Get here when it was the cluster-differentiator drop-down that was changed.
        $cluster_differentiator = $form_state['triggering_element']['#value'];
      }
    }
    $depth = 0;
    $field = field_info_field($cluster_differentiator);

    if (!empty($field['settings']['allowed_values'])) {
      // Possibly a taxonomy or list
      foreach ($field['settings']['allowed_values'] as $tree) {
        if ($vocabulary = taxonomy_vocabulary_machine_name_load($tree['vocabulary'])) {
          if ($terms = taxonomy_get_tree($vocabulary->vid, $tree['parent'])) {
            foreach ($terms as $term) {
              $depth = max($term->depth, $depth);
            }
            break;
          }
        }
      }
    }
    $description = empty($depth)
      ? t('Below enter the zoom level range to be associated with this differentiator.')
      : t('Below enter the zoom level ranges to which each of the region hierarchy levels apply.');
    $form['cluster_differentiator']['cluster_differentiator_field'] = array(
      '#title' => t('Marker cluster differentiator'),
      '#type' => 'select',
      '#options' => $fields,
      '#default_value' => $cluster_differentiator,
      '#ajax' => array(
        'callback' => '_ip_geoloc_plugin_style_leaflet_refresh_cluster_fieldset_js',
        'wrapper' => 'cluster-differentiator-wrapper',
        'effect' => 'fade',
        'speed' => 'fast',
      ),
      '#description' => $description . '<br/>' . t('Zoom ranges must not overlap.')
    );
    $depth = ($field['type'] === 'addressfield') ? 4 : $depth + 1;
    $zoom_range_defaults = array(
      1 => '6--7',
      2 => '8--11',
      3 => '12--14',
      4 => '15--18'
    );
    if ($field['type'] === 'addressfield') {
      $title = array(
        1 => t('Zoom range for country'),
        2 => t('Zoom range for administrative area (state, district, county)'),
        3 => t('Zoom range for locality (city, town, village)'),
        4 => t('Zoom range for post code (ZIP)'),
      );
    }
    else {
      for ($level = 1; $level <= $depth; $level++) {
        $title[$level] = t('Zoom range for %field level @level', array(
          '%field' => $fields[$cluster_differentiator],
          '@level' => $level
        ));
      }
    }
    for ($level = 1; $level <= $depth; $level++) {
      $zoom_range = $this->options['cluster_differentiator']['zoom_ranges'][$level];
      $form['cluster_differentiator']['zoom_ranges'][$level] = array(
        '#type' => 'textfield',
        '#title' => $title[$level],
        '#size' => 28,
        '#default_value' => $zoom_range,
        '#element_validate' => array('ip_geoloc_range_widget_validate'),
      );
      if (isset($zoom_range_defaults[$level])) {
        $form['cluster_differentiator']['zoom_ranges'][$level]['#description'] =
          t('Typical range: @range', array('@range' => $zoom_range_defaults[$level]));
      }
    }
    $form['cluster_differentiator']['zoom_ranges'][$depth]['#description'] .=
      '<br/>' . t('The maximum zoom level for this map can be found below under <strong>More map options</strong>.');
  }

  /**
   * Validate the options form.
   */
  public function options_validate(&$form, &$form_state) {
    ip_geoloc_plugin_style_bulk_of_form_validate($form, $form_state);
    $map_height = $form_state['values']['style_options']['map_height'];
    if (!is_numeric($map_height) || $map_height <= 0) {
      form_error($form['map_height'], t('Map height cannot be blank and must be a positive number.'));
    }
    $selected_map = ip_geoloc_plugin_style_leaflet_map_get_info($form_state['values']['style_options']['map']);
    $zoom_top = 18;
    if (isset($selected_map['settings']['maxZoom'])) {
      $zoom_top = $selected_map['settings']['maxZoom'];
    }
    $max_zoom = $form_state['values']['style_options']['map_options']['maxzoom'];
    if ($max_zoom != '' && (!is_numeric($max_zoom) || $max_zoom < 0 || $max_zoom > $zoom_top)) {
      form_error($form['map_options']['maxzoom'], t('Maximum zoom level for %map must be in range 0..@zoomtop', array(
        '%map' => $selected_map['label'], '@zoomtop' => $zoom_top)));
    }
    $zoom = $form_state['values']['style_options']['map_options']['zoom'];
    if ($zoom != '' && (!is_numeric($zoom) || $zoom < 0 || $zoom > $max_zoom)) {
      form_error($form['map_options']['zoom'], t('Initial zoom level must be a non-negative number less than or equal to Maximum zoom level.'));
    }
  }

  /**
   * Transform the View result in a list of marker locations and render on map.
   *
   * @todo refactor
   */
  public function render() {
    if (empty($this->options['map']) || !($map = ip_geoloc_plugin_style_leaflet_map_get_info($this->options['map']))) {
      return t('No Leaflet map was selected or map configuration was not found.');
    }
    if (!empty($this->view->live_preview)) {
      return t('The preview function is incompatible with Leaflet maps so cannot be used. Please visit the page path or the block to view your map.');
    }

    ip_geoloc_plugin_style_render_fields($this);
    $locations = ip_geoloc_plugin_style_extract_locations($this);

    $this->fill_out_location_regions($locations);

    $marker_color = $this->options['default_marker']['default_marker_color'];
    $visitor_marker_color = $this->options['visitor_marker']['visitor_marker_color'];
    $center_option = !isset($this->options['center_option']) ? 0 : $this->options['center_option'];
    $cluster_radius = $this->options['cluster_radius'];
    $tag_css_classes = $this->options['tags']['tag_css_class'];

    $module_path = drupal_get_path('module', 'ip_geoloc');
    $marker_path = file_create_url(variable_get('ip_geoloc_marker_directory', $module_path . '/markers'));

    $max_zoom = (int) $this->options['map_options']['maxzoom'];
    $zoom = (int) $this->options['map_options']['zoom'];
    $scroll_wheel_zoom = (bool) $this->options['map_options']['scrollwheelzoom'];
    $dragging = (bool) $this->options['map_options']['dragging'];

    if (!empty($locations) &&
        ($center_option == IP_GEOLOC_MAP_CENTER_ON_FIRST_LOCATION ||
        ($visitor_marker_color == 'none' && count($locations) == 1))) {
      $map['center'] = _ip_geoloc_get_center(reset($locations));
    }
    elseif (($center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS || $center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS_WEIGHTED) && !empty($locations)) {
      list($center_lat, $center_lon) = ip_geoloc_center_of_locations($locations, $center_option == IP_GEOLOC_MAP_CENTER_OF_LOCATIONS_WEIGHTED);
      $map['center'] = array(
        'lat' => $center_lat,
        'lon' => $center_lon,
      );
    }

    uasort($locations, '_ip_geoloc_plugin_style_leaflet_compare');

    $visitor_location = ip_geoloc_get_visitor_location();
    if (!isset($visitor_location['latitude'])) {
      $visitor_location = db_query('SELECT * FROM {ip_geoloc} WHERE ip_address = :ip_address', array(':ip_address' => ip_address()))->fetchAssoc();
    }
    
    if ((empty($locations) || $center_option == IP_GEOLOC_MAP_CENTER_ON_VISITOR) && isset($visitor_location['latitude'])) {
      $map['center'] = array(
        'lat' => $visitor_location['latitude'],
        'lon' => $visitor_location['longitude'],
      );
    }
    if (empty($locations)) {
      $ll = trim($this->options['empty_map_center']);
      if (empty($ll)) {
        // No map whatsoever.
        return;
      }
      if ($ll != t('visitor')) {
        // Empty map centered on coordinates provided.
        list($map['center']['lat'], $map['center']['lon']) = preg_split("/[\s,]+/", $ll);
      }
      // Else Empty map centered on visitor location, as set above.
    }

    $marker_dimensions = explode('x', variable_get('ip_geoloc_marker_dimensions', '21 x 34'));
    $marker_width  = (int) $marker_dimensions[0];
    $marker_height = (int) $marker_dimensions[1];

    switch (variable_get('ip_geoloc_marker_anchor_pos', 'bottom')) {
      case 'top':
        $marker_anchor = 0;
        break;

      case 'middle':
        $marker_anchor = (int) (($marker_height + 1) / 2);
        break;

      default:
        $marker_anchor = $marker_height;
    }

    $features = array();
    foreach ($locations as $location) {
      $feature = array();
      if (isset($location->latitude) || isset($location->lat)) {
        $feature['type'] = 'point';
        $feature['lat'] = isset($location->latitude) ? $location->latitude : $location->lat;
        $feature['lon'] = isset($location->longitude) ? $location->longitude : $location->lon;
      }
      elseif (isset($location->component)) {
        // Possibly parsed by leaflet_process_geofield()
        // see _ip_geoloc_plugin_style_extract_lat_lng().
        $feature['type'] = $location->type;
        $feature['component'] = $location->component;
      }
      elseif (isset($location->points)) {
        $feature['type'] = $location->type;
        $feature['points'] = $location->points;
      }

      if (isset($location->id)) {
        // Allow marker events to identify the corresponding node.
        $feature['feature_id'] = $location->id;
      }

      // At this point $feature['type'] should be set.
      if (!empty($feature['type']) && $feature['type'] != 'point') {
        // Linestring, polygon ...
        $feature['flags'] = LEAFLET_MARKERCLUSTER_EXCLUDE_FROM_CLUSTER;
      }
      elseif (!isset($feature['lat'])) {
        // Points must have coords.
        continue;
      }
      $feature['popup'] = $location->balloon_text;

      if (!empty($location->marker_special_char) || !empty($location->marker_special_char_class)) {
        $has_special_markers = TRUE;
        if (!empty($location->marker_special_char)) {
          $feature['specialChar'] = $location->marker_special_char;
        }
        if (!empty($location->marker_special_char_class)) {
          $feature['specialCharClass'] = $location->marker_special_char_class;
        }
      }
      elseif (!empty($this->options['default_marker']['default_marker_special_char'])
        || !empty($this->options['default_marker']['default_marker_special_char_class'])) {
        $has_special_markers = TRUE;
        $feature['specialChar'] = $this->options['default_marker']['default_marker_special_char'];
        $feature['specialCharClass'] = $this->options['default_marker']['default_marker_special_char_class'];
      }
      if (!empty($location->marker_tooltip)) {
        $has_special_markers = TRUE;
        $feature['tooltip'] = $location->marker_tooltip;
      }
      if (!empty($location->regions)) {
        // Make sure we start with 0 or regions will come across as Object
        // rather than array.
        $feature['regions'] = array(0 => '') + $location->regions;
        $has_special_markers = TRUE;
        $feature['tooltip'] = empty($feature['tooltip']) ? '' : $feature['tooltip'] . '<br/>';
        $second_last = count($feature['regions']) - 2;
        if ($second_last > 0) {
          $feature['tooltip'] = $feature['regions'][$second_last] . ' ';
        }
        $feature['tooltip'] .= end($feature['regions']);
      }
      if (!empty($location->marker_tag)) {
        $has_special_markers = TRUE;
        $feature['tag'] = $location->marker_tag;
      }
      if (!empty($tag_css_classes)) {
        $feature['cssClass'] = $tag_css_classes;
      }
      if ((isset($location->marker_color) && _ip_geoloc_is_no_marker($location->marker_color)) ||
          (!isset($location->marker_color) && _ip_geoloc_is_no_marker($marker_color))) {
        // "No marker" as opposed to "default" marker.
        $has_special_markers = TRUE;
        $feature['icon'] = FALSE;
      }
      elseif (!empty($location->marker_color) || !empty($marker_color)) {
        // Switch from default icon to specified color.
        $color = empty($location->marker_color) ? $marker_color : $location->marker_color;
        $feature['icon'] = array(
          'iconUrl' => $marker_path . "/$color.png",
          'iconSize' => array('x' => $marker_width, 'y' => $marker_height),
          'iconAnchor' => array('x' => (int) (($marker_width + 1) / 2), 'y' => $marker_anchor),
          // Just above topline, center.
          'popupAnchor' => array('x' => 0, 'y' => -$marker_height - 1),
        // 'shadowUrl'
        // 'shodowSize'
        // 'shadowAnchor
        );
      }
      $features[] = $feature;
    }
    if ($visitor_marker_color != 'none' && isset($visitor_location['latitude'])) {
      // See leaflet/README.txt for examples of Leaflet "features"
      $visitor_feature = array(
        'type' => 'point',
        'lat' => $visitor_location['latitude'],
        'lon' => $visitor_location['longitude'],
        'specialChar' => $this->options['visitor_marker']['visitor_marker_special_char'],
        'specialCharClass' => $this->options['visitor_marker']['visitor_marker_special_char_class'],
        'popup' => t('Your approximate location'),
        'tooltip' => t('Your approximate location'),
        // See leaflet_markercluster.drupal.js.
        'flags' => LEAFLET_MARKERCLUSTER_EXCLUDE_FROM_CLUSTER,
      );
      if ($visitor_marker_color != '') {
        if (!empty($visitor_feature['specialChar']) || !empty($visitor_feature['specialCharClass'])) {
          $has_special_markers = TRUE;
        }
        $visitor_feature['icon'] = array(
          'iconUrl' => $marker_path . "/$visitor_marker_color.png",
          'iconSize' => array('x' => $marker_width, 'y' => $marker_height),
          'iconAnchor' => array('x' => (int) (($marker_width + 1) / 2), 'y' => $marker_anchor),
          // Just above topline, center.
          'popupAnchor' => array('x' => 0, 'y' => -$marker_height - 1),
        );
      }
      $features[] = $visitor_feature;
    }
    // If auto-box is chosen ($center_option==0), zoom only when there are
    // 0 or 1 markers [#1863374]
    if (empty($center_option) && count($features) > 1) {
      unset($map['settings']['zoom']);
    }
    else {
      $map['settings']['zoom'] = $zoom;
    }
    $map['settings']['maxZoom'] = $max_zoom;
    $map['settings']['scrollWheelZoom'] = $scroll_wheel_zoom;
    $map['settings']['dragging'] = $dragging;

    if (!empty($cluster_radius)) {
      $map['settings']['maxClusterRadius'] = $cluster_radius;
      $map['settings']['addRegionToolTips'] = TRUE;
      $map['settings']['animateAddingMarkers'] = TRUE;
    }
   
    // Make sure we start array with 0 and no missing elements. Ohterwise this
    // array will arrive as an Object on the JS side.
    $map['settings']['regionLevels'] = array_fill(0, $max_zoom + 1, 0);
    if (!empty($this->options['cluster_differentiator']['zoom_ranges'])) {
      foreach ($this->options['cluster_differentiator']['zoom_ranges'] as $level => $zoom_range) {
        for ($zoom = 1; $zoom <= $max_zoom; $zoom++) {
          if (ip_geoloc_is_in_range($zoom, $zoom_range)) {
            $map['settings']['regionLevels'][$zoom] = $level;
          }
        }
      }
    }
    // Remove any gaps and zeroes.
    for ($z = 1; $z <= $max_zoom; $z++) {
      if ($map['settings']['regionLevels'][$z] == 0) {
        $map['settings']['regionLevels'][$z] = $map['settings']['regionLevels'][$z - 1];
      }
    }
    // See [#1802732].
    $map_id = 'ip-geoloc-map-of-view-' . $this->view->name . '-' . $this->display->id . '-' . md5(serialize($features));

    drupal_add_js(drupal_get_path('module', 'leaflet') . '/leaflet.drupal.js');
    $settings = array(
      'mapId' => $map_id,
      'map' => $map,
      'features' => $features,
    );
    $options = array(
      'type' => 'setting',
      'scope' => 'footer', // 'footer' only works for type 'inline'
    );
    drupal_add_js(array('leaflet' => array($settings)), $options);

    // Little hacky this, but can't see another way to load libraries for
    // Leaflet More Maps, Leaflet MarkerCluster, Leaflet Hash...
    libraries_load('leaflet');
    drupal_alter('leaflet_map_prebuild', $settings);

    if (!empty($cluster_radius) && !module_exists('leaflet_markercluster')) {
      $display_name = $this->view->get_human_name() . ' (' . $this->display->display_title . ')';
      drupal_set_message(t('Cannot cluster markers in View %display_name - Leaflet MarkerCluster module not enabled.', array('%display_name' => $display_name)), 'warning');
    }
    if (!empty($has_special_markers)) {
      // Load the CSS that comes with the font icon library which in return
      // tells the browser to fetch either the WOFF, TTF or SVG files that
      // define the font faces.
      drupal_add_library('ip_geoloc', 'ip_geoloc_font_icon_libs');
      drupal_add_css($module_path . '/css/ip_geoloc_leaflet_markers.css');
      drupal_add_js($module_path . '/js/ip_geoloc_leaflet_tagged_marker.js');
    }

    $output = theme('ip_geoloc_leaflet', array(
      'map_id' => $map_id,
      'height' => $this->options['map_height'],
      'view' => $this->view,
    ));
    return $output;
  }

  private function _fill_out_location_region($field_type, &$location, $region) {

    switch ($field_type) {

      case 'taxonomy_term_reference':
        $region_hierarchy = array_reverse(taxonomy_get_parents_all($region));
        // Leaf term (i.e. highest cluster level) is first in hierarchy.
        $level = 1;
        foreach ($region_hierarchy as $region_term) {
          $location->regions[$level++] = $region_term->name;
        }
        break;
 
      case 'addressfield':
        $format = array();
        $context = array('mode' => NULL);
        addressfield_format_address_generate($format, $region, $context);
        $location->regions = array(
          1 => empty($format['country']['#options'][$region['country']]) ? '' : $format['country']['#options'][$region['country']],
          2 => empty($format['locality_block']['administrative_area']['#options'][$region['administrative_area']]) ? '': $format['locality_block']['administrative_area']['#options'][$region['administrative_area']],
          3 => empty($region['locality']) ? '' : $region['locality'],
          4 => empty($region['postal_code']) ? '' : $region['postal_code'],
        );
        break;

      default:
        $location->regions[1] = $region;
    }
  }

  public function fill_out_location_regions($locations) {
    $cluster_differentiator = $this->options['cluster_differentiator']['cluster_differentiator_field'];
    if ($field = field_info_field($cluster_differentiator)) {
      foreach ($this->view->result as $key => $row) {
        if (isset($locations[$key])) {
          $values = ip_geoloc_get_view_result($this, $field, $key);
          if (!empty($values)) {
            $region = ($field['type'] == 'addressfield') ? $values : end($values);
            $this->_fill_out_location_region($field['type'], $locations[$key], $region);
          }
        }
      }
    }
  }
}

/**
 * Ajax callback in response to new rows or the diff. drop-down being changed.
 *
 * At this point the $form has already been rebuilt. All we have to do here is
 * tell AJAX what part of the browser form needs to be updated.
 */
function _ip_geoloc_plugin_style_leaflet_refresh_cluster_fieldset_js($form, &$form_state) {
  // Return the updated fieldset, so that ajax.inc can issue commands to the
  // browser to update only the targeted sections of the page.
  return $form['options']['style_options']['cluster_differentiator'];
}

/**
 * Get the center of a lat/lon pair.
 */
function _ip_geoloc_get_center($location) {
  if (empty($location->type) || $location->type == 'point') {
    $lat = isset($location->lat) ? $location->lat : (isset($location->latitude) ? $location->latitude : 0.0);
    $lon = isset($location->lon) ? $location->lon : (isset($location->longitude) ? $location->longitude : 0.0);
    return array('lat' => $lat, 'lon' => $lon);
  }
  if (!empty($location->component[0]['points'][0])) {
    return $location->component[0]['points'][0];
  }
}

/**
 * Checks if marker color is a good value.
 *
 * @param mixed $marker_color
 *   The color of the marker.
 *
 * @return bool
 *   TRUE if marker color is "0", zero, or FALSE
 *   FALSE if marker color equals '' or NULL
 */
function _ip_geoloc_is_no_marker($marker_color) {
  return isset($marker_color) && ($marker_color === '0' || $marker_color === 0 || $marker_color === FALSE);
}

/**
 * Wrapper around the only programmatic dependency we have on Leaflet module.
 *
 * Note: this indirectly calls ip_geoloc_leaflet_map_info_alter($map_info).
 */
function ip_geoloc_plugin_style_leaflet_map_get_info($map_name = NULL) {
  return leaflet_map_get_info($map_name);
}

/**
 * Callback to compare locations based on weight.
 */
function _ip_geoloc_plugin_style_leaflet_compare($location1, $location2) {
  $weight1 = empty($location1->weight) ? 0 : $location1->weight;
  $weight2 = empty($location2->weight) ? 0 : $location2->weight;
  return $weight2 - $weight1;
}
