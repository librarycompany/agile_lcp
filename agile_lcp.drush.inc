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
      'new' => array(
        'description' => dt('Whether this is the first run of the harvest, Defaults to FALSE'),
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
      'url' => array(
        'value' => 'optional',
        'description' => dt("The url to initiate harvesting.  Defaults to 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21'"),
      ),
      'from' => array(
        'description' => dt("Datestamp of first harvested object - optional (YYYY-MM-DD)"),
        'required' => FALSE,
      ),
      'to' => array(
        'description' => dt("Datestamp of first harvested object - optional (YYYY-MM-DD)"),
        'required' => FALSE,
      ),
    ),
    'drupal dependencies' => array(
      'islandora',
    ),
    'examples' => array(
      'drush -u 1 OAI_Prep',
    ),
    'aliases' => array('OAI_Prep'),
  );

  $items['agile_get_collection_pids'] = array(
    'description' => dt('Gets and records OAI collection PIDS.'),
    'options' => array(
      'limit' => array(
        'description' => dt("Number of records to retrieve at once"),
        'required' => FALSE,
      ),
      'iterations' => array(
        'description' => dt("number of times to iterate"),
        'required' => FALSE,
      ),
    ),
    'drupal dependencies' => array(
      'islandora',
    ),
    'examples' => array(
      'drush -u 1 get_collection_pids',
    ),
    'aliases' => array('GCP'),
  );
  $items['agile_remove_indexed_collection_pids'] = array(
    'description' => dt('Removes already indexed pids from database.'),
    'examples' => array(
      'drush -u 1 UCPT',
    ),
    'aliases' => array('UCPT'),
  );
  $items['agile_activate'] = array(
    'description' => dt('Activates objects from file'),
    'examples' => array(
      'drush -u 1 AA',
    ),
    'aliases' => array('AA'),
  );
  return $items;
}

/**
 * Kicks off serises of sub-batches to harvest all OAI records from token.
 */
function drush_agile_lcp_agile_OAI_harvest_batch() {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $batch = array();
  $limit = drush_get_option('limit', 1);
  $new = drush_get_option('new', FALSE);

  if ($new) {
    $limit--;
    $batch['operations'][] = array(
      'agile_lcp_add_OAI_batch_worker',
      array(
        'token_id' => NULL,
        'token' => NULL,
      ),
    );
  }

  $tokens = agile_lcp_get_unproccessed_tokens($limit);
  $count = count($tokens);
  if ($count < $limit) {
    $limit = count($tokens);
    drush_print(dt("There are only @count tokens left to process", array('@count' => $count)));
  }
  foreach ($tokens as $token_id => $token) {
    $batch['operations'][] = array(
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

/**
 * KIcks off sub_batch.
 * 
 * @param string $token_id
 *   token id
 * @param string $token
 *   resumption_token
 * @param context $context
 *   Batch context.
 */
function agile_lcp_add_OAI_batch_worker($token_id = NULL, $token = NULL, &$context) {
  drush_print('.', 0, NULL, FALSE);
  if ($token) {
    $url = "http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&resumptionToken=$token";
  }
  else {
    drush_print(dt("First run."));
    $url = 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21';
  }
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
  if ($token) {
    $batch['operations'][] = array('agile_lcp_drush_mark_token', array(
        'token_id' => $token_id,
        'token' => $token,
      ));
  }
  batch_set($batch);
}

/**
 * Builds and ingests islandora object for  OAI record.
 *
 * @param String $identifier
 *   Unique OAI identifier
 */
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

  try {
    islandora_add_object($object);
  }
  catch (Exception $ex) {
    drush_print("Failed to ingest $identifier");
  }
}

/**
 * Output for feedback for screen.
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
 * @param int $token_id
 *   Unique identifier for token
 * @param string $token
 *   Token being updated
 */
function agile_lcp_drush_mark_token($token_id, $token) {
  drush_print("$token_id complete.");
  $query = db_update('resumption_token')
          ->condition('token', $token)
          ->fields(array(
            'processed' => 1,
          ))->execute();
}

/**
 * Adds all tokens from supplied URL to prepare for harvesting.
 */
function drush_agile_lcp_agile_OAI_prep_batch() {
  drush_print(dt("Adding to database -"));
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $url = drush_get_option('url', 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21');
  $to = drush_get_option('to', FALSE);
  $from = drush_get_option('from', FALSE);
  if ($to) {
    $url .= "&to=$to";
  }
  if ($from) {
    $url .= "&from=$from";
  }
  set_time_limit(0);
  $url_parts = explode('?', $url);
  $base_url = $url_parts[0] . "?verb=ListIdentifiers&resumptionToken=";
  $data = file_get_contents($url);
  if (!$data) {
    drush_print(dt("No results returned - please check url and login credentials"));
    return;
  }
  $dom = new DOMDocument();
  $dom->loadXML($data);
  $resumption_node = $dom->getElementsByTagName('resumptionToken')->item(0);
  $token = $resumption_node->nodeValue;
  $count = 1;
  while ($token) {
    $new_url = $base_url . $token;
    $token = NULL;
    $data = file_get_contents($new_url);
    $sub_dom = new DOMDocument();
    $sub_dom->loadXML($data);
    $resumption_tokens = $sub_dom->getElementsByTagName('resumptionToken');
    if ($resumption_tokens->length > 0) {
      $count++;
      $resumption_node = $sub_dom->getElementsByTagName('resumptionToken')->item(0);
      $token = $resumption_node->nodeValue;
      agile_lcp_add_token($token);
      if ($count % 10 == 0) {
        drush_print('.', 0, NULL, FALSE);
      }
    }
  }
  drush_print(dt("\n Complete."));
}

/**
 * Gets all collection pids for resource index.
 */
function drush_agile_lcp_agile_get_collection_pids() {
  $result = db_select('collection_pids', 'cp')
      ->fields('cp')
      ->execute();
  $count = $result->rowCount();
  drush_print("$count in database table - start");
  $limit = drush_get_option('limit', 1000);
  $iterations = drush_get_option('iterations', 1);
  $tuque = new IslandoraTuque();
  for ($i = 0; $i < $iterations; $i++) {
    $offset = ($i * $limit) + $count;
    $query = <<<EOQ
PREFIX fm: <info:fedora/fedora-system:def/model#>
SELECT  ?pid 
FROM <RISEARCH>
WHERE
{
      ?pid  <fedora-rels-ext:isMemberOfCollection> <info:fedora/islandora:imported_oai>

 }
        limit  $limit
        offset  $offset
EOQ;
    $results = $tuque->repository->ri->sparqlQuery($query);
    if (count($results) == 0) {
      drush_print(dt("No more repository results"));
      break;
    }
    foreach ($results as $result) {
      $nid = db_insert('collection_pids')
          ->fields(array(
            'pid' => $result['pid']['value'],
            'indexed' => 0,
          ))
          ->execute();
    }
  }
  $result = db_select('collection_pids', 'cp')
      ->fields('cp')
      ->execute();
  $count = $result->rowCount();
  drush_print("$count in database table - end");
}

/**
 * One time funciton to remove processed PIDS from database table.
 */
function drush_agile_lcp_agile_remove_indexed_collection_pids() {
  $result = db_select('collection_pids', 'cp')
      ->fields('cp')
      ->condition('indexed', 0)
      ->execute();
  $count = $result->rowCount();
  drush_print("$count unindexed in database table - start");
  $file = "/tmp/collectionPIDS.txt";
  $handle = fopen($file, "r");
  if ($handle) {
    while (($line = fgets($handle)) !== FALSE) {
      $pid = trim($line);
      $query = db_update('collection_pids')
              ->condition('pid', $pid)
              ->fields(array(
                'indexed' => 1,
              ))->execute();
    }

    fclose($handle);
  }
  else {
    drush_print("Could not open file");
  }
  $result = db_select('collection_pids', 'cp')
      ->fields('cp')
      ->condition('indexed', 0)
      ->execute();
  $count = $result->rowCount();
  drush_print("$count unindexed in database table - end");
}
