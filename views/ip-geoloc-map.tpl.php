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
  * - $show_visitor_location, center the map on a green marker depicting the
  *   visitor's current location
  */
?>
<div class="ip-geoloc-map view-based-map">
  <?php echo ip_geoloc_output_map_multi_location($locations, $div_id, $map_options, $map_style, $show_visitor_location); ?>
</div>