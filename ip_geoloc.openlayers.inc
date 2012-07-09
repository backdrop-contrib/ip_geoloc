<?php

/*
 * @file
 * ip_geoloc.openlayers.inc
 *
 * Hook implementations for the OpenLayers interface.
 */

DEFINE('IP_GEOLOC_MARKER_LAYER', 'ip_geoloc_marker_layer');

/**
 * Implements hook_openlayers_map_preprocess_alter($map)
 *
 * Add or alter map before main processing. Add styles, behaviors, layers....
 * Called from openlayers_build_map(), which in turn is called by function
 * openlayers_render_map_data().
 * At this point center and zoom level have already been initialised from the
 * Map configuration page.
 */
function ip_geoloc_openlayers_map_preprocess_alter(&$map) {

  if (isset($_SESSION['ip_geoloc']['location']['latitude'])) {
    $longitude = $_SESSION['ip_geoloc']['location']['longitude'];
    $latitude = $_SESSION['ip_geoloc']['location']['latitude'];
    $map['center']['initial']['centerpoint'] = "$longitude,$latitude";
  }
//$map['center']['initial']['zoom'] = 5;

  // Introduce a layer that will be created in ip_geoloc_openlayers_layers().
  $map['layers'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;
  $map['layer_activated'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;
  $map['layer_switcher'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;
}

/**
 * Implementation of hook_ctools_plugin_api().
 *
 * Required to add a layer to OpenLayers, see ip_geoloc_openlayers_layers().
 */
function ip_geoloc_ctools_plugin_api($module, $api) {
  if ($module == 'openlayers' /*&& $api == 'openlayers_layers'*/) {
    return array('version' => 1);
  }
}

/**
 * Implements hook_openlayers_layers().
 *
 * Called via ctools in the heart of openlayers_build_map().
 * @see
 */
function ip_geoloc_openlayers_layers() {
  // Attempt to load information from cache.
  // For now use a arg-based check for skipping cache.
  if (arg(0) != 'admin') {
    $cache = cache_get(IP_GEOLOC_MARKER_LAYER);
    if (isset($cache->data)) {
      return $cache->data;
    }
  }

  $layers = array();
  $layer = new stdClass();
  $layer->api_version = 1;
  $layer->name = IP_GEOLOC_MARKER_LAYER;
  $layer->title = 'Location markers';
  $layer->description = 'A layer used by module ip_geoloc for location markers extracted from a view.';
  $layer->data = array(
    'layer_type' => 'openlayers_layer_type_raw',
    'baselayer' => FALSE,
    'projection' => array('900913')
  );
  $layers[$layer->name] = $layer;

  cache_set(IP_GEOLOC_MARKER_LAYER, $layers);

  return $layers;
/*


  $views = views_get_all_views();

  // Find views that use the "Map (Openlayers, via IP Geolocation)" format,
  // aka the 'ip_geoloc_openlayers' style-plugin and create a basic layer to
  // hold location markers (aka features). We can't add the markers here, as
  // this function isn't passed any argumentes, so we don't know for which view
  // we're creating this layer!
  foreach ($views as $view) {
    foreach ($view->display as $display => $data) {
      $view->set_display($display);
      if (!empty($data->display_options['style_plugin']) && $data->display_options['style_plugin'] == 'ip_geoloc_openlayers') {
        //Build Layer
        $layer = new stdClass();
        $layer->api_version = 1;
        $layer->name = $view->name . '_' . $display;
        $layer->title = !empty($view->display['default']->handler->options['title']) ?
          $view->display['default']->handler->options['title'] :
          (empty($view->human_name) ? $view->name : $view->human_name);
        $layer->description = $view->description . ' - ' . $data->display_title;
        $layer->data = array(
          'layer_type' => 'openlayers_views_vector', // openlayers_layer_type_raw
          'projection' => array('4326'), //array('900913'),
          'features' => array(
             array(
                "wkt" => "POINT(65 33)",
                "projection"=> "4326",
                "attributes"=> array(
                  "name"=> "Afghanistan",
                  "description"=> "248"
                )
              )
          ),
          'baselayer' => FALSE,
          'type' => 'Vector',
          'url' => array(),
          'options' => array(),
          'events' => array(),
          'views' => array('view' => $view->name, 'display' => $display), // needed?
        );
        $layers[$layer->name] = $layer;
      }
    }
    $view->destroy();
  }
  return $layers;
 */
}

/**
 * Implements hook_openlayers_map_alter().
 *
 * Called at the end of openlayers_build_map().
 */
function ip_geoloc_openlayers_map_alter(&$map) {
  return;
}