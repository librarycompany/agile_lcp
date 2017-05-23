<?php

/**
 * @file
 * Defines the form that allows users to harvest.
 */

/**
 * Defines the form that allows users to prep for harvest.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function agile_lcp_prep_harvest($form, $form_state) {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $token_count = db_select('resumption_token')
      ->fields(NULL, array('field'))
      ->condition('processed', 0)
      ->countQuery()->execute()
      ->fetchField();
  
  if (!$token_count) {
    $message = t('All tokens will be entered into database for processing');
  }
  else {
    $message = t("Job has been prepared. There are @count tokens in the database", array('@count' => $token_count));
  }
  $form = array();
  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => "<p>$message</p>",
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Prepare harvest"),
  );
  return $form;
}

function agile_lcp_prep_harvest_submit($form, $form_state) {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $url = 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21';
  $tokens = array();
  set_time_limit(0);
  $urlParts = explode('?', $url);
  $base_url = $urlParts[0] . "?verb=ListIdentifiers&resumptionToken=";
  $data = file_get_contents($url);
  if (!$data) {
    drupal_set_message("No results returned - please check url and login credentials", 'warning');
    return;
  }
  $dom = new DOMDocument();
  $dom->loadXML($data);
  $resumption_node = $dom->getElementsByTagName('resumptionToken')->item(0);
  $token = $resumption_node->nodeValue;
  while ($token) {
    $new_url = $base_url . $token;
    $data = file_get_contents($new_url);
    $sub_dom = new DOMDocument();
    $sub_dom->loadXML($data);
    $resumption_tokens = $sub_dom->getElementsByTagName('resumptionToken');
    if ($resumption_tokens->length > 0) {
      $resumption_node = $sub_dom->getElementsByTagName('resumptionToken')->item(0);
      $token = $resumption_node->nodeValue;
      agile_lcp_add_token($token);
    }
  }
}