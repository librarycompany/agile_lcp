<?php

/**
 * @file
 * Defines the form that allows users to harvest.
 */

/**
 * Defines the form that allows users to harvest.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function agile_lcp_harvest_form($form, &$form_state) {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $token_count = db_select('resumption_token')
          ->fields(NULL, array('field'))
          ->condition('processed', 0)
          ->countQuery()->execute()->fetchField();

  $intro = t("There are @count tokens to process", array('@count' => $token_count));
  $token = agile_lcp_get_next_token_id();
  if (!$token) {
    $message = t('No tokens processed yet');
  }
  else {
    $message = t('Next token ID to process is @token', array('@token' => $token));
  }
  $form = array();
  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => "<p>$intro</p><p>$message</p>",
  );

  $form['is_new'] = array(
    '#title' => t("First run?"),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#access' => $token == 1,
  );
  $form['process'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#title' => 'Import from OAI',
    '#description' => t('Number of tokens to process'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Import via OAI"),
  );
  return $form;
}

/**
 * Submit handler.
 *
 * @param array $form
 *   Drupal form
 * @param array $form_state
 *   Drupal form state
 */
function agile_lcp_harvest_form_submit($form, &$form_state) {
  $iterations = $form_state['values']['process'];
  $batch = array(
    'title' => t('Importing records'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'agile_lcp') . '/includes/harvest_form.inc',
    'progress_message' => t('@current of @total operations completed.'),
  );

  if (isset($form_state['values']['is_new']) && $form_state['values']['is_new']) {
    $batch['operations'][] = array('agile_lcp_import_oai', array(
        'token_id' => NULL,
        'token' => NULL,
      ));
    $iterations--;
  }
  $tokens = agile_lcp_get_unproccessed_tokens($iterations);
  foreach ($tokens as $id => $redemption_token) {
    $batch['operations'][] = array('agile_lcp_import_oai', array(
        'token_id' => $id,
        'token' => $redemption_token,
      ));
  }
  batch_set($batch);
  batch_process();
}

/**
 * Builds Islandora objects from OAI input.
 *
 * @param string $identifier
 *   OAI unique indentifier
 */
function agile_lcp_build_object($identifier) {
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $existing = agile_lcp_get_pids_from_identifier($identifier);
  $streams = agile_get_streams_from_entry($identifier);
  if ($existing) {
    foreach ($existing as $pid) {
      $object = islandora_object_load($pid);
      foreach ($streams as $dsid => $content) {
        if ($object[$dsid]) {
          $object[$dsid]->content = $content;
        }
      }
    }
    return;
  }
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $object = $repository->constructObject('islandora');
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:oai_importCModel');
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:imported_oai');
  $dom = new DOMDocument();
  $dom->loadXML($streams['MODS']);
  $non_sort = '';
  $nonSort_nodes = $dom->getElementsByTagName('nonSort');
  if ($nonSort_nodes->length > 0) {
    $non_sort = $nonSort_nodes->item(0)->nodeValue . ' ';
  }
  $title = $dom->getElementsByTagName('title')->item(0)->nodeValue;
  $label = $non_sort . $title;
  $object->label = $label;
  foreach ($streams as $dsid => $content) {
    $type = ($dsid == 'DC') ? 'X' : 'M';
    $mime = ($dsid == 'DC') ? 'text/xml' : 'application/xml';
    $ds = $object->constructDatastream($dsid, 'M');
    $ds->setContentFromString($content, FALSE);
    $ds->label = $dsid;
    $ds->mimetype = $mime;
    $object->ingestDatastream($ds);
  }
  islandora_add_object($object);
}

/**
 * Initiates batch.
 */
function agile_lcp_import_oai($token_id = FALSE, $token = FALSE) {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  if ($token) {
    $url = "http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&resumptionToken=$token";
  }
  else {
    $url = 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21';
  }
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $new_identifiers = $dom->getElementsByTagName('identifier');
  foreach ($new_identifiers as $new_identifier) {
    $identifiers[] = $new_identifier->nodeValue;
  }
  $batch2 = array(
    'title' => t('Building Islandora Objects'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'agile_lcp') . '/includes/harvest_form.inc',
    'progress_message' => t('@current of @total operations completed.'),
  );
  foreach ($identifiers as $identifier) {
    $batch2['operations'][] = array('agile_lcp_build_object', array(
        'identifier' => $identifier,
      ),
    );
  }
  if ($token) {
    $batch2['operations'][] = array('agile_lcp_mark_token', array(
        'token' => $token,
      ),
    );
  }
  batch_set($batch2);
}

/**
 * Marks token as processed.
 *
 * @param string $token
 *   token being preocessed.
 */
function agile_lcp_mark_token($token) {
  $query = db_update('resumption_token')
          ->condition('token', $token)
          ->fields(array(
            'processed' => 1,
          ))->execute();
}
