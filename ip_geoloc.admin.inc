<?php

/**
 * @file
 * Admin configuration settings for IP Geolocation.
 */

/**
 * Menu callback for admin settings.
 */
function ip_geoloc_admin_configure() {

  if (ip_geoloc_diagnose() > 0) {
    // Form for sync-ing the geolocation table with the system accesslog
    $form['ip_geoloc_db_options'] = array(
      '#type' => 'fieldset',
      '#title' => 'Update geolocation database using past visitor IP addresses from system access log',
      '#description'  => t('You can update the geolocation database in batches by pressing the button below. As a rough guide, count on a 1 minute wait for every 100 IP addresses. Add another 2 minutes per 100 IP addresses if you ticked the option to employ the Google Maps API to reverse-geocode to street addresses. If your server interrupts the process you can continue from where it stopped by refreshing this page and pressing the button again. You will not lose any data.')
    );
    $form['ip_geoloc_db_options']['ip_geoloc_sync_with_accesslog'] = array(
      '#type' => 'submit',
      '#value' => t('Update now'),
      '#submit' => array('ip_geoloc_sync_with_accesslog'),
    );
    $form['ip_geoloc_db_options']['ip_geoloc_sync_batch_size'] = array(
      '#type' => 'textfield',
      '#size' => 4,
      '#title' => 'Batch size',
      '#default_value'  => variable_get('ip_geoloc_sync_batch_size', 500),
      '#description' => t('To change the default batch size, press "Save configuration".')
    );
  }

  $form['ip_geoloc_data_collection_options'] = array(
    '#type' => 'fieldset',
    '#title' => 'Data collection options',
  );
  $form['ip_geoloc_data_collection_options']['ip_geoloc_google_to_reverse_geocode'] = array(
    '#type' => 'checkbox',
    '#title' => 'Employ the Google Maps API to reverse-geocode lat/long coordinates to street addresses.',
    '#default_value' => variable_get('ip_geoloc_google_to_reverse_geocode', TRUE),
    '#description' => t('For present and future visitors this is done via the client-side version of the Maps API. For the upload of historic visitor location data the server-side version of the Maps API is used. Note that Google may impose a limit on the daily number of API calls coming from the same machine.')
  );
  $form['ip_geoloc_data_collection_options']['ip_geoloc_smart_ip_as_backup'] = array(
    '#type' => 'checkbox',
    '#title' => 'Employ Smart IP as a backup.',
    '#default_value' => variable_get('ip_geoloc_smart_ip_as_backup', TRUE),
    '#description' => t('This refers to situations where the lat/long coords could not be established (e.g. because the browser/device is not supported or the user declined us access to their location) or the Google Maps API address lookup failed or was not employed through the tick box above.')
  );
  $form['ip_geoloc_data_collection_options']['ip_geoloc_location_check_interval'] = array(
    '#type' => 'textfield',
    '#title' => t('Mininum elapsed time in seconds before geolocation data for the same user will be collected again.'),
    '#default_value' => variable_get('ip_geoloc_location_check_interval', IP_GEOLOC_LOCATION_CHECK_INTERVAL),
    '#description' => t('Geolocation information associated with an IP address may change over time, for instance when the visitor is using a mobile device and is moving.')
  );

  $form['ip_geoloc_advanced'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => 'Advanced options',
    '#description' => t('')
  );
  $form['ip_geoloc_advanced']['ip_geoloc_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Detail execution progress with status messages.'),
    '#default_value' => variable_get('ip_geoloc_debug', FALSE),
    '#description' => t('The above tick box applies to the administrator (uid=1) only.')
  );
  $form['ip_geoloc_advanced']['ip_geoloc_erase_session'] = array(
    '#type' => 'submit',
    '#value' => t('Erase geolocation data from session now'),
    '#submit' => array('ip_geoloc_erase_session'),
  );
  $form['ip_geoloc_advanced']['ip_geoloc_erase_db'] = array(
    '#type' => 'submit',
    '#value' => t('Erase entire geolocation database now'),
    '#submit' => array('ip_geoloc_erase_db'),
  );

  return system_settings_form($form);
}