<?php

/**
 * @file
 * Implementation of Drush hooks.
 */

/**
 * Implements hook_drush_help().
 */
function agile_lcp_drush_help($command) {
  switch ($command) {
    case 'drush:agile_OAI_harvest_batch':
      return dt('Creates objects');
  }
}

/**
 * Implements hook_drush_command().
 */
function agile_lcp_drush_command() {
  $items = array();
  $items['agile_OAI_harvest_batch'] = array(
    'description' => dt('Harvests records from OAI stream.'),
    'options' => array(
      'limit' => array(
        'description' => dt('The number of batches to process.  Defaults to 1'),
        'required' => FALSE,
      ),
    ),
    'aliases' => array('OAI'),
    'drupal dependencies' => array(
      'islandora',
    ),
    'examples' => array(
      'drush -u 1 OAI --limit=3',
    ),
  );


  $items['agile_OAI_prep_batch'] = array(
    'description' => dt('Preps batch process to harvest records from OAI stream.'),
    'options' => array(
      'limit' => array(
        'value' => 'optional',
        'description' => dt('The number of batches to process.  Defaults to 1'),
      ),
    ),
    'drupal dependencies' => array(
      'islandora',
    ),
    'examples' => array(
      'rush -u 1 OAI --limit=3',
    ),
    'aliases' => array('OAI_Prep'),
  );


  return $items;
}

function drush_agile_lcp_agile_OAI_harvest_batch() {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $limit = drush_get_option('limit', 1);

  $batch = array();
  $tokens = agile_lcp_get_unproccessed_tokens($limit);
  foreach ($tokens as $token_id => $token) {
    $batch ['operations'][] = array(
      'agile_lcp_add_OAI_batch_worker',
      array(
        'token_id' => $token_id,
        'token' => $token,
      ),
    );
  }
  drush_print("Start batch. Processing $limit tokens");
  batch_set($batch);
  drush_backend_batch_process();
}

function agile_lcp_add_OAI_batch_worker($token_id, $token, &$context) {
  drush_print('.', 0, NULL, FALSE);
  $url = "http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&resumptionToken=$token";
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $new_identifiers = $dom->getElementsByTagName('identifier');

  $batch = array();
  $batch['operations'][] = array('agile_lcp_batch_notifier', array(
      'token_id' => $token_id,
  ));
  foreach ($new_identifiers as $new_identifier) {
    $batch['operations'][] = array('agile_lcp_drush_build_object', array(
        'identifier' => $new_identifier->nodeValue,
    ));
  }
  $batch['operations'][] = array('agile_lcp_drush_mark_token', array(
      'token_id' => $token_id,
      'token' => $token,
  ));
  batch_set($batch);
}

function agile_lcp_drush_build_object($identifier) {
  drush_print('.', 0, NULL, FALSE);
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $streams = get_streams_from_entry($identifier);
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
    $mine = ($dsid == 'DC') ? 'text/xml' : 'application/xml';
    $ds = $object->constructDatastream($dsid, 'M');
    $ds->setContentFromString($content, FALSE);
    $mime_detect = new MimeDetect();
    $ds->label = $dsid;
    $ds->mimetype = 'text/xml';
    $object->ingestDatastream($ds);
  }
  islandora_add_object($object);
}

/**
 * Output for feedback fro screen.
 *
 * @param strng $token_id
 *   OAI token
 */
function agile_lcp_batch_notifier($token_id) {
  drush_print("\n Start sub_batch - Processing token #$token_id");
}

/**
 * Marks token as processed.
 *
 * @param string $token
 */
function agile_lcp_drush_mark_token($token_id, $token) {
  drush_print("$token_id complete.");
  $query = db_update('resumption_token')
          ->condition('token', $token)
          ->fields(array(
            'processed' => 1,
          ))->execute();
}
