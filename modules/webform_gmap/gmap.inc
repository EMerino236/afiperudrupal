<?php

/**
 * Implements _webform_defaults_component().
 */
function _webform_defaults_gmap() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'value' => NULL,
    'extra' => array(
      'private' => FALSE,
      'description' => '',
    ),
  );
}

/**
 * Implements _webform_theme_component().
 */
function _webform_theme_gmap() {
  return array(
    'webform_gmap' => array(
      'render element' => 'element',
      'file' => 'gmap.inc',
    ),
    'webform_display_gmap' => array(
      'render element' => 'element',
      'file' => 'gmap.inc',
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_gmap($component) {
  $form = array();
  $defaults = gmap_defaults();
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
  );
  $form['extra']['center'] = array(
    '#type' => 'textfield',
    '#title' => t('Default center'),
    '#default_value' => isset ($component['extra']['center']) ? $component['extra']['center'] : $defaults['latlong'],
    '#description' => t('The default value of the field.'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
  );
  // TODO: add default zoom
  return $form;
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_gmap($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#type' => 'webform_gmap',
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#title_display' => $component['extra']['title_display'] ? $component['extra']['title_display'] : 'before',
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#description' => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#element_validate' => array('webform_validate_gmap'),
    '#default_value' => $filter ? _webform_filter_values($component['value'], NULL, NULL, NULL, FALSE) : $component['value'],
    '#process' => array('webform_expand_gmap'),
    '#theme' => 'webform_gmap',
    '#theme_wrappers' => array('webform_element'),
    '#pre_render' => array('webform_element_title_display'),
    '#post_render' => array('webform_element_wrapper'),
    '#webform_component' => $component,
  );

  // Set the value from Webform if available.
  if (!empty($value['latitude'])) {
    $element['#default_value'] = $value;
  }

  return $element;
}


/**
 * Form API #process function for Webform gmap fields.
 */
function webform_expand_gmap($element) {
  if (is_array ($element['#default_value']))
    $default_values = $element['#default_value'];
  else
    $default_values = array ('latitude' => '', 'longitude' => '');
  $element['latitude'] = array(
    '#type' => 'hidden',
    '#title' => t ('Latitude'),
    '#default_value' => $default_values['latitude'],
  );
  $element['longitude'] = array(
    '#type' => 'hidden',
    '#title' => t ('Longitude'),
    '#default_value' => $default_values['longitude'],
  );

  $latlong = explode(',', $element['#webform_component']['extra']['center']);

  $settings = array(
    'id' => gmap_get_auto_mapid(),
    'latitude' => $latlong[0],
    'longitude' => $latlong[1],
    'markers' => array (
      array(
        'latitude' => $latlong[0],
        'longitude' => $latlong[1],
        'name' => $element['#webform_component']['form_key'],
        'markername' => '',
        'offset' => 0,
        'text' => t ('Please drag me to the right beach'),
        'autoclick' => true,
        'opts' => array ('draggable' => true),
      )),
  );
  $element['map'] = array(
    '#value' => theme('gmap', array('#settings' => $settings)),
  );
  drupal_add_js (drupal_get_path ('module', 'webform_gmap') . '/webform_gmap.js');

  return $element;
}


/**
 * Theme a webform gmap element.
 */
function theme_webform_gmap($element) {

  // Add error classes to all items within the element.
  if (form_get_error($element)) {
    $element['latitude']['#attributes']['class'] = array('error');
    $element['longitude']['#attributes']['class'] = array('error');
  }

  $output = '<div class="webform-container-inline">' . drupal_render($element['latitude']) . drupal_render($element['longitude']) . '</div>';
  $output .= drupal_render($element['map']);

  return $output;
}

function webform_validate_gmap($element, $form_state) {
  // TODO
}

/**
 * Implements _webform_submit_component().
 */

/**
 * Implements _webform_display_component().
 */
function _webform_display_gmap($component, $value, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_gmap',
    '#theme_wrappers' => $format == 'html' ? array('webform_element', 'webform_element_wrapper') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#format' => $format,
    '#value' => $value,
    '#pre_render' => array('webform_element_title_display'),
    '#webform_component' => $component,
  );
}

/**
 * Format the output of entered data for this component.
 */
function theme_webform_display_gmap($element) {
  $output = $element['#value']['latitude'] . ',' . $element['#value']['longitude'];
  return $output;
}


/**
 * Implements _webform_analysis_component().
 */
function _webform_analysis_gmap($component, $sids = array()) {
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (" . implode(",", $placeholders) . ")" : "";
  $query = 'SELECT no,data ' .
    ' FROM {webform_submitted_data} ' .
    ' WHERE nid = %d ' .
    ' AND  cid = %d ' . $sidfilter .
    ' ORDER BY sid ASC ';

  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));

  $times = array();
  $submissions = 0;
  while ($row = db_fetch_array($result)) {
    $submissions++;
    if ($row['data']) {
      $times[] = webform_date_array($row['data']);
    }
  }

  // Display stats.
  $nonblanks = count($times);
  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  return $rows;
}


/**
 * Implements _webform_table_component().
 */
function _webform_table_gmap($component, $value) {
  if ($value['latitude']) {
    return $value['latitude'] . ', ' . $value['longitude'];
  }
  else {
    return '';
  }
}

/**
 * Implements _webform_csv_headers_component().
 */
function _webform_csv_headers_gmap($component, $export_options) {
  $header = array();
  $header[0][] = '';
  $header[1][] = '';
  $header[2] = array ($component['name'] . ' latitude', $component['name'] . ' longitude');
  return $header;
}

/**
 * Implements _webform_csv_data_component().
 */
function _webform_csv_data_gmap($component, $export_options, $value) {
  if ($value['latitude']) {
    return array ($value['latitude'], $value['longitude']);
  }
  else {
    return array ('', '');
  }
}
