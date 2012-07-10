<?php

/*
 * @file
 * ip_geoloc.openlayers.inc
 *
 * Hook implementations for the OpenLayers interface.
 */

DEFINE('IP_GEOLOC_MARKER_LAYER', 'ip_geoloc_marker_layer');
DEFINE('IP_GEOLOC_VISITOR_MARKER_LAYER', 'ip_geoloc_visitor_marker_layer');

/**
 * Implements hook_openlayers_map_preprocess_alter($map)
 *
 * Add or alter map before main processing. Add styles, behaviors, layers....
 * Called from openlayers_build_map().
 * At this point center, zoom level etc. have already been initialised from the
 * Map configuration page.
 */
function ip_geoloc_openlayers_map_preprocess_alter(&$map) {

//$map['center']['initial']['zoom'] = 5;

  // Introduce a couple of layers that will be created below in function
  // ip_geoloc_openlayers_layers().
  $map['layers'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;
  $map['layer_activated'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;

  $map['layers'][IP_GEOLOC_VISITOR_MARKER_LAYER] = IP_GEOLOC_VISITOR_MARKER_LAYER;

  // Markers may be switched on/off via a tickbox on the rendered map, if so
  // configured on this page: /admin/structure/openlayers/maps/<map_name>/edit,
  // vertical tab "Layers & Styles", bottom section "Overlay layers"
  // $map['layer_switcher'][IP_GEOLOC_MARKER_LAYER] = IP_GEOLOC_MARKER_LAYER;
}

/**
 * Implementation of hook_ctools_plugin_api().
 *
 * Required to add a layer to OpenLayers, see ip_geoloc_openlayers_layers().
 */
function ip_geoloc_ctools_plugin_api($module, $api) {
  if ($module == 'openlayers' /* && $api == 'openlayers_layers' */) {
    return array('version' => 1);
  }
}

/**
 * Implements hook_openlayers_layers().
 *
 * Called via ctools in the heart of openlayers_build_map().
 * This function has no arguments, so layers are added unconditionally, unless
 * globals are used.
 */
function ip_geoloc_openlayers_layers() {
  /* Hardly worth caching this and shouldn't as user's position may change.
  if (arg(0) != 'admin') {
    $cache = cache_get(IP_GEOLOC_MARKER_LAYER);
    if (isset($cache->data)) {
      return $cache->data;
    }
  } */
  $layers = array();

  $marker_layer = new stdClass();
  $marker_layer->api_version = 1;
  $marker_layer->name = IP_GEOLOC_MARKER_LAYER;
  $marker_layer->title = 'Location markers';
  $marker_layer->description = "Layer to map a view's locations.";
  $marker_layer->data = array(
    'baselayer' => FALSE,
    'layer_type' => 'openlayers_layer_type_raw',
    'projection' => array('900913')
  );
  // Features, ie. location markers, are added based on the view output, see
  // file ip_geoloc_plugin_style_openlaysers.inc
  $layers[$marker_layer->name] = $marker_layer;

  $visitor_layer = new stdClass();
  $visitor_layer->api_version = 1;
  $visitor_layer->name = IP_GEOLOC_VISITOR_MARKER_LAYER;
  $visitor_layer->title = 'Current visitor marker';
  $visitor_layer->description = "Layer to mark current visitor's position.";
  $visitor_layer->data = array(
    'baselayer' => FALSE,
    'layer_type' => 'openlayers_layer_type_raw',
    'projection' => array('900913')
  );
  // Add the visitor's location as a single feature in its own layer, so
  // that it may be styled (eg marker colour) and activated separately on page
  // admin/structure/openlayers/maps/<map-name>/edit
  if (isset($_SESSION['ip_geoloc']['location']['longitude'])) {
    $longitude = $_SESSION['ip_geoloc']['location']['longitude'];
    $latitude  = $_SESSION['ip_geoloc']['location']['latitude'];
    $visitor_layer->data['features'][] = array(
      'attributes' => array('title_rendered' => t('Your retrieved location.')),
      'projection' => '4326',
      'wkt' => "POINT($longitude $latitude)"
    );
  }
  $layers[$visitor_layer->name] = $visitor_layer;

  //cache_set(IP_GEOLOC_MARKER_LAYER, $layers);
  return $layers;
}

/**
 * Implements hook_openlayers_map_alter().
 *
 * Called at the end of openlayers_build_map().
 */
function ip_geoloc_openlayers_map_alter(&$map) {
  return;
}