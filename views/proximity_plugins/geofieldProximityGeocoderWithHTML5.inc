<?php

/**
 * @file
 * Contains geofieldProximityGeocoderWithHTML5.
 */

class geofieldProximityGeocoderWithHTML5 extends geofieldProximityGeocoder {

  public function option_definition(&$options, $views_plugin) {
    //parent::option_definition($options, $views_plugin);
    $options['geofield_proximity_with_HTML5_share_exposed'] = array('default' => '');
  }

  public function value_form(&$form, &$form_state, $views_plugin) {
    parent::value_form($form, $form_state, $views_plugin);
    $note = t('You may enter %me to indicate your current location.', array(
      '%me' => t('me'),
    ));
    $form['value']['#description'] = $note;
  }

  public function value_validate(&$form, &$form_state, $views_plugin) {
    $values = $form_state['values']['options']['value'];
    if (!is_numeric($values['distance']) || $values['distance'] < 0) {
      form_set_error(NULL, t('Not a valid distance.'));
    }
  }

  public function options_form(&$form, &$form_state, $views_plugin) {
    parent::options_form($form, $form_state, $views_plugin);

    $exposed_geofields = array();
    $filters = $views_plugin->view->display_handler->handlers['filter'];
    foreach ($filters as $name => $filter) {
      if ($filter->definition['handler'] == 'geofield_handler_filter' && !empty($filter->options['exposed'])) {
        $exposed_geofields[$name] = $filter->ui_name();
      }
    }
    $options = array('' => '-- ' . t("no don't do that") . ' --') + $exposed_geofields;

    $form['geofield_proximity_with_HTML5_share_exposed'] = array(
      '#type' => 'select',
      '#title' => t('Inherit filter values from the exposed filter belonging to the field below'),
      '#options' => $options,
      '#default_value' => $views_plugin->options['geofield_proximity_with_HTML5_share_exposed'],
      '#states' => array(
        'visible' => array(
          ':input[name="options[expose_button][checkbox][checkbox]"]' => array('checked' => FALSE),
          ':input[name="options[source]"]' => array('value' => 'geocoder_with_HTML5'),
        ),
      ),
    );
    $form['value']['#states'] = array(
      'visible' => array(':input[name="options[geofield_proximity_with_HTML5_share_exposed]"]' => array('value' => '')),
    );
  }

  public function options_validate(&$form, &$form_state, $views_plugin) {
    parent::options_validate($form, $form_state, $views_plugin);
  }

  public function getSourceValue($views_plugin) {
    if (!empty($views_plugin->options['geofield_proximity_with_HTML5_share_exposed'])) {

      $share_from = $views_plugin->options['geofield_proximity_with_HTML5_share_exposed'];
      if (isset($views_plugin->view->exposed_raw_input[$share_from])) {
        $exposed_filter = $views_plugin->view->exposed_raw_input[$share_from];  
        $views_plugin->value['origin'] = $exposed_filter['origin'];
        $views_plugin->value['unit'] = $exposed_filter['unit'];
        $views_plugin->value['distance'] = $exposed_filter['distance'];
        if (isset($exposed_filter['distance2'])) {
          $views_plugin->value['distance2'] = $exposed_filter['distance2'];
        }
      }
    }
    if (empty($exposed_filter)) {
      $exposed_filter = isset($views_plugin->options['value']) ? $views_plugin->options['value'] : $views_plugin->options['geofield_proximity_geocoder'];
    }

    $origin = isset($exposed_filter['origin']) ? trim($exposed_filter['origin']) : '';

    if (drupal_strlen($origin) <= 1 || $origin === 'me' || $origin === t('me')) {
      $location = ip_geoloc_get_visitor_location();
      if (isset($location['latitude'])) {
        return array(
          'latitude' => $location['latitude'],
          'longitude' => $location['longitude']
        );
      }
      ip_geoloc_debug('IGPV&M proximity: visitor location temporarily not available.', 'warning');
      return FALSE;
    }
    return parent::getSourceValue($views_plugin);
  }
}
