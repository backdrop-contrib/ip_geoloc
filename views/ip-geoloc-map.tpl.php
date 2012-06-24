<?php
 /**
  * This template is used to output a map of marker locations taken from a view.
  *
  * Variables available:
  * - $view: the view object, if needed
  * - $locations: array of locations each with lat/long coordinates and balloon
  *   texts; the map will normally be auto-centered on the visitor's current
  *   location, however, if not requrested or not found the first location in
  *   the array will be used to center the map
  * - $div_id: id of the div in which the map will be injected, arbitrary but
  *   must be unique
  * - $map_options: passed to Google Maps API, eg '{"mapTypeId":"roadmap", "zoom": 10}'
  * - $map_style: CSS style string, like 'height: 200px; width: 500px'
  * - $center_option, one of:
  *   0: no center override (must be provided thorugh $map_options)
  *   1: auto-center the map on the first location in the $locations array
  *   2: auto-center the map on a distinct marker depicting the visitor's current location
  */
?>
<div class="ip-geoloc-map view-based-map">
  <?php echo ip_geoloc_output_map_multi_location($locations, $div_id, $map_options, $map_style, $center_option); ?>
</div>