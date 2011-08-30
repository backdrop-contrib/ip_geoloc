<?php

/**
 * @file
 * Admin configuration settings for IP Geolocation.
 */

/**
 * Menu callback for admin settings.
 */
function ip_geoloc_admin_configure() {

  $form['ip_geoloc_data_collection_options'] = array(
    '#type' => 'fieldset',
    '#title' => 'Data collection options',
    '#description' => t('')
  );
  $form['ip_geoloc_data_collection_options']['ip_geoloc_google_to_reverse_geocode'] = array(
    '#type' => 'checkbox',
    '#title' => 'Employ the Google Maps API to reverse-geocode lat/long coordinates to street addresses.',
    '#default_value' => variable_get('ip_geoloc_google_to_reverse_geocode', TRUE),
    '#description' => t('Note that Google may impose a limit on the daily number of API calls coming from the same machine.')
  );
  $form['ip_geoloc_data_collection_options']['ip_geoloc_location_check_min_elapsed'] = array(
    '#type' => 'textfield',
    '#title' => t('Mininum elapsed time in seconds before geolocation data for the same user will be collected again.'),
    '#default_value' => variable_get('ip_geoloc_location_check_min_elapsed', 30),
    '#description' => t('Geolocation information associated with an IP address may change over time, for instance when the visitor is using a mobile device and is moving.')
  );

  $geoloc_count = db_query('SELECT COUNT(DISTINCT ip_address) FROM {ip_geoloc}')->fetchField();
  drupal_set_message(t("The geolocation database currently contains information for %geoloc_count IP addresses.", array('%geoloc_count' => $geoloc_count)), 'status', FALSE);
  if (!db_table_exists('accesslog')) {
    drupal_set_message(t('The <strong>accesslog</strong> database table does not exist, probably because the Statistics module is not installed. Consequently historic IP geolocation information cannot be genereated for IP addresses of users that visited the site before IP Geolocation was enabled. The "Visitor log" view will not be available until you enable the <strong>Statistics</strong> module and its access log. The "Current visitor" block is not affected.'), 'warning');
  }
  else {
    $ip_address_count = db_query('SELECT COUNT(DISTINCT hostname) FROM {accesslog}')->fetchField();
    drupal_set_message(t("The system access log currently contains entries from %ip_address_count IP addresses.", array('%ip_address_count' => $ip_address_count)), 'status', FALSE);
    if (!variable_get('statistics_enable_access_log', FALSE)) {
      drupal_set_message(t('The <strong>Statistics</strong> module is enabled, but its system access log is not. Therefore the "Visitor log" view is frozen in time and will not grow. The "Current visitor" block is not affected. You can enable the access log at <em>Configuration >> Statistics</em>.'), 'warning');
    }
    else {
      $count = count(ip_geoloc_ips_to_be_synched());
      if ($count == 0) {
        drupal_set_message(t("The geolocation database is up to date and in sync with the system access log."), 'status', FALSE);
      }
      else {
        drupal_set_message(t("%count IP addresses in the system access log currently have no associated lat/long or address information on the geolocation database.", array('%count' => $count)), 'warning', FALSE);
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
    }
  }

  $form['ip_geoloc_data_advanced'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => 'Advanced options',
    '#description' => t('')
  );
  $form['ip_geoloc_data_advanced']['ip_geoloc_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Detail execution progress with status messages while collecting and storing geolocation data.'),
    '#default_value' => variable_get('ip_geoloc_debug', FALSE),
    '#description' => t('')
  );
  $form['ip_geoloc_data_advanced']['ip_geoloc_erase_session'] = array(
    '#type' => 'submit',
    '#value' => t('Erase geolocation data from session now'),
    '#submit' => array('ip_geoloc_erase_session'),
  );
  $form['ip_geoloc_data_advanced']['ip_geoloc_erase_db'] = array(
    '#type' => 'submit',
    '#value' => t('Erase entire geolocation database now'),
    '#submit' => array('ip_geoloc_erase_db'),
  );

  return system_settings_form($form);
}