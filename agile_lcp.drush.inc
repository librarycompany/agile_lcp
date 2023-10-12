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
  $items = [];

  $items['agile_OAI_harvest_batch'] = [
    'description' => dt('Harvests records from OAI stream.'),
    'options' => [
      'limit' => [
        'description' => dt('The number of batches to process.  Defaults to 1'),
        'required' => FALSE,
      ],
      'new' => [
        'description' => dt('Whether this is the first run of the harvest, Defaults to FALSE'),
        'required' => FALSE,
      ],
    ],
    'aliases' => ['OAI'],
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 OAI --limit=3',
    ],
  ];

  $items['agile_OAI_harvest_one'] = [
    'description' => dt('Harvests one record from the OAI source.'),
    'options' => [
      'identifier' => [
        'description' => dt('The Koha Biblio Number of the MARC record to harvest from OIA.'),
        'required' => TRUE,
      ],
    ],
    'aliases' => ['OAI_1'],
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 OAI_1 --identifier=197559'
    ]
  ];

  $items['agile_OAI_prep_batch'] = [
    'description' => dt('Preps batch process to harvest records from OAI stream.'),
    'options' => [
      'url' => [
        'value' => 'optional',
        'description' => dt("The url to initiate harvesting.  Defaults to 'https://librarycompany.kohacatalog.com/cgi-bin/koha/oai.pl?verb=ListIdentifiers&metadataPrefix=marc21'"),
      ],
      'reset' => [
        'description' => dt("Set to 1 if you want to restart the harvest using the parameters; otherwise the harvest will continue where it last left off."),
        'required' => FALSE,
      ],
      'from' => [
        'description' => dt("Datestamp of first harvested object - optional (YYYY-MM-DD)"),
        'required' => FALSE,
      ],
      'to' => [
        'description' => dt("Datestamp of first harvested object - optional (YYYY-MM-DD)"),
        'required' => FALSE,
      ],
    ],
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 OAI_Prep',
    ],
    'aliases' => ['OAI_Prep'],
  ];

  $items['agile_get_collection_pids'] = [
    'description' => dt('Gets and records OAI collection PIDS.'),
    'options' => [
      'limit' => [
        'description' => dt("Number of records to retrieve at once"),
        'required' => FALSE,
      ],
      'iterations' => [
        'description' => dt("number of times to iterate"),
        'required' => FALSE,
      ],
    ],
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 get_collection_pids --limit=10',
    ],
    'aliases' => ['GCP'],
  ];
  $items['agile_remove_indexed_collection_pids'] = [
    'description' => dt('Removes already indexed pids from database.'),
    'examples' => [
      'drush -u 1 UCPT',
    ],
    'aliases' => ['UCPT'],
  ];
  $items['agile_activate'] = [
    'description' => dt('Activates objects from file'),
    'examples' => [
      'drush -u 1 AA',
    ],
    'aliases' => ['AA'],
  ];
  $items['agile_lcp_solr_reindex'] = [
    'description' => dt('Reindexes PIDS from RI to solr'),
    'options' => [
      'iterations' => [
        'description' => dt("number of times to iterate"),
        'required' => FALSE,
      ],
    ],
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 SOLR_INDEX --limit=10',
    ],
    'aliases' => ['SOLR_INDEX'],
  ];
  $items['mark_solr_pids'] = [
    'description' => dt('Takes all pids form solr and updates cp table'),
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 mark_solr',
    ],
    'aliases' => ['mark_solr'],
  ];
  $items['remove_duplicates'] = [
    'description' => dt('Removes duplicated OAI objects'),
    'drupal dependencies' => [
      'islandora',
    ],
    'examples' => [
      'drush -u 1 remove_dupes',
    ],
    'aliases' => ['remove_dupes'],

  ];
  return $items;
}

function drush_agile_lcp_agile_OAI_harvest_one() {
  $identifier = drush_get_option('identifier', '');

  if ($identifier != '') {
    agile_lcp_drush_build_object($identifier);
  }
}

/**
 * Kicks off series of sub-batches to harvest all OAI records from token.
 */
function drush_agile_lcp_agile_OAI_harvest_batch() {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $batch = [];
  $limit = drush_get_option('limit', 1);
  $new = drush_get_option('new', FALSE);

  if ($new) {
    $limit--;
    $batch['operations'][] = [
      'agile_lcp_add_OAI_batch_worker',
      [
        'token_id' => NULL,
        'token' => NULL,
      ],
    ];
  }

  $tokens = agile_lcp_get_unproccessed_tokens($limit);
  $count = count($tokens);
  if ($count < $limit) {
    $limit = count($tokens);
    drush_print(dt("There are only @count tokens left to process", ['@count' => $count]));
  }
  foreach ($tokens as $token_id => $token) {
    $batch['operations'][] = [
      'agile_lcp_add_OAI_batch_worker',
      [
        'token_id' => $token_id,
        'token' => $token,
      ],
    ];
  }
  drush_print("Start batch. Processing $limit tokens");
  batch_set($batch);
  drush_backend_batch_process();
}

/**
 * Kicks off sub_batch.
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
    $url = "https://librarycompany.kohacatalog.com/cgi-bin/koha/oai.pl?verb=ListIdentifiers&resumptionToken=$token";
  }
  else {
    drush_print(dt("First run."));
    $url = 'https://librarycompany.kohacatalog.com/cgi-bin/koha/oai.pl?verb=ListIdentifiers&metadataPrefix=marc21';
  }
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $new_identifiers = $dom->getElementsByTagName('identifier');

  $batch = [];
  $batch['operations'][] = [
    'agile_lcp_batch_notifier',
    [
      'token_id' => $token_id,
      'token' => $token,
    ],
  ];
  foreach ($new_identifiers as $new_identifier) {
    $batch['operations'][] = [
      'agile_lcp_drush_build_object',
      [
        'identifier' => $new_identifier->nodeValue,
      ],
    ];
  }
  if ($token) {
    $batch['operations'][] = [
      'agile_lcp_drush_mark_token',
      [
        'token_id' => $token_id,
        'token' => $token,
      ],
    ];
  }
  batch_set($batch);
}

/**
 * Builds and ingests islandora object for OAI record.
 *
 * @param string $identifier
 *   Unique OAI identifier
 */
function agile_lcp_drush_build_object(string $identifier) {
  drush_print('Loading ' . $identifier);
  module_load_include('inc', 'agile_lcp', 'includes/utilities');

  $streams = agile_get_streams_from_entry($identifier);
  $existing = agile_lcp_get_pids_from_identifier($identifier);

  // since Koha updates, main identifier is the Koha biblio number
  // try finding a matching record using the 001 control number
  $marc = $streams['MARC'];
  $dom = new DOMDocument();
  $dom->loadXML($marc);
  $xpath = new DOMXPath($dom);
  $xpath->registerNamespace('marc', "http://www.loc.gov/MARC21/slim");

  $query = "//marc:controlfield[@tag = '001']";
  $identifiers = $xpath->query($query);
  if ($identifiers->length) {
    $old_identifier = $identifiers->item(0)->nodeValue;
  } else {
    $old_identifier = '999999999';
  }
  
  $existing2 = agile_lcp_get_pids_from_identifier($old_identifier);

  // Streams are not returned if OAI record has been deleted.
  if (!$streams) {
    if(!$existing && !$existing2) {
      drush_print(" Record $identifier has deleted status.  Not added");
    }
    else {
      // Remove current OAI records, if record has been deleted since original ingest.
      $oai_records = agile_lcp_get_pids_from_identifier($identifier, TRUE);
      $tuque = new IslandoraTuque();
      foreach ($oai_records as $pid) {
        $tuque->repository->purgeObject($pid);
        drush_print(" Object $pid  with identifier $identifier has been deleted.");
      }
    }
    return;
  }

  if ($existing ) {
    foreach ($existing as $pid) {
      $object = islandora_object_load($pid);
      foreach ($streams as $dsid => $content) {
        if ($object[$dsid]) {
          $object[$dsid]->content = $content;
        }
      }
      drush_print("Updated $pid.  Identifier = $identifier");
    }
    if (!$existing2) return;
  }

  if ($existing2 ) {
    foreach ($existing2 as $pid) {
      $object = islandora_object_load($pid);
      foreach ($streams as $dsid => $content) {
        if ($object[$dsid]) {
          $object[$dsid]->content = $content;
        }
      }
      drush_print("Updated $pid.  Identifier = $identifier");
    }
    return;
  }

  /* Discover layer is disabled, so don't add any new records */

  /*
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
    $ds = $object->constructDatastream($dsid, $type);
    $ds->setContentFromString($content, FALSE);
    $ds->label = $dsid;
    $ds->mimetype = $mime;
    $object->ingestDatastream($ds);
  }

  try {
    islandora_add_object($object);
    drush_print("Added {$object->id}.  Identifier = $identifier");
  } catch (Exception $ex) {
    drush_print("Failed to ingest $identifier");
  }
  */
}

/**
 * Output for feedback for screen.
 *
 * @param string $token_id
 *   OAI token
 */
function agile_lcp_batch_notifier($token_id, $token) {
  drush_print("\n Start sub_batch - Processing token #$token_id - $token");
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
    ->fields([
      'processed' => 1,
    ])->execute();
}

/**
 * Adds all tokens from supplied URL to prepare for harvesting.
 */
function drush_agile_lcp_agile_OAI_prep_batch() {
  drush_print(dt("Adding to database -"));
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $url = drush_get_option('url', 'https://librarycompany.kohacatalog.com/cgi-bin/koha/oai.pl?verb=ListIdentifiers&metadataPrefix=marc21');
  $to = drush_get_option('to', FALSE);
  $from = drush_get_option('from', FALSE);
  $reset = drush_get_option('reset', FALSE);
  if ($to) {
    $url .= "&to=$to";
  }
  if ($from) {
    $url .= "&from=$from";
  }
  set_time_limit(0);
  $url_parts = explode('?', $url);
  $base_url = $url_parts[0] . "?verb=ListIdentifiers&resumptionToken=";

  if ($reset == '1') {
    // Load the initial URL to get the first resumption token; note that this doesn't process the first set of records!
    $data = file_get_contents($url);
    if (!$data) {
      drush_print(dt("No results returned - please check url and login credentials"));
      return;
    }
    $dom = new DOMDocument();
    $dom->loadXML($data);
    $resumption_node = $dom->getElementsByTagName('resumptionToken')->item(0);
    $token = $resumption_node ? $resumption_node->nodeValue : NULL;
  } else {
    // get the last resumption token from the database
    $token = agile_lcp_get_last_processed_token();
    // need to reprocess this token, because it could have additional records in this set
    agile_lcp_add_token($token);
    drush_print("Resuming from last token in database: $token.");
  }

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
      $resumption_node = $sub_dom->getElementsByTagName('resumptionToken')
        ->item(0);
      $token = $resumption_node->nodeValue;
      agile_lcp_add_token($token);
      //if ($count % 10 == 0) {
        drush_print('.', 0, NULL, FALSE);
      //}
    }
  }
  drush_print(dt("\n Complete."));
}

/**
 * Gets all pids from resource index and puts them in database.
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
    $offset = ($i * $limit);
    $query = <<<EOQ
PREFIX fm: <info:fedora/fedora-system:def/model#>
SELECT  distinct ?s 
FROM <RISEARCH>
WHERE
{
      ?s  fm:state fm:Active .
      ?s fm:hasModel ?o
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
      $pid = $result['s']['value'];
      drush_print($pid);
      $nid = db_merge('collection_pids')
        ->key(['pid' => $pid])
        ->fields([
          'pid' => $pid,
          'indexed' => 0,
        ])
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
 * One time function to remove processed PIDS from database table.
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
        ->fields([
          'indexed' => 1,
        ])->execute();
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

function drush_agile_lcp_solr_reindex() {
  $iterations = drush_get_option('iterations', 1);
  drush_print($iterations);
  for ($i = 1; $i <= $iterations; $i++) {
    $start = microtime(TRUE);
    $results = db_select('collection_pids', 'cp')
      ->fields('cp')
      ->condition('indexed', 0)
      ->range(0, 1000)
      ->execute()
      ->fetchAll();
    if (!$results) {
      drush_print("All pids have been indexed");
      break;
    }
    foreach ($results as $result) {
      $pid = $result->PID;
      $curl = curl_init();
      $url = "http://localhost:8080/fedoragsearch/rest?operation=updateIndex&action=fromPid&value=$pid";
      curl_setopt_array($curl, [
        CURLOPT_RETURNTRANSFER => 1,
        CURLOPT_URL => $url,
        CURLOPT_USERPWD => 'fedoraAdmin:DXhn8eg4zU6Z6NgA',
        //CURLOPT_USERPWD => 'fedoraAdmin:fedoraAdmin',
      ]);
      $success = curl_exec($curl);
      curl_close($curl);
      if ($success != FALSE) {
        $num_updated = db_update('collection_pids')
          ->fields([
            'indexed' => 1,

          ])
          ->condition('pid', $pid)
          ->execute();
      }
      else {
        drush_print("Failure on $pid");
      }
    }
    drush_print(microtime(TRUE) - $start);
  }

}

function drush_agile_lcp_mark_solr_pids() {
  $query = '*:*';
  $limit = 4000;
  $start = 0;
  for ($i = 0; $i < 86; $i++) {
    $qp = new IslandoraSolrQueryProcessor();
    $qp->buildQuery('*:*');
    $qp->solrParams['fq'] = $query;
    $qp->solrStart = $start;
    $qp->solrParams['fl'] = 'PID';
    $qp->solrLimit = $limit;
    $qp->executeQuery(FALSE);
    $objects = $qp->islandoraSolrResult['response']['objects'];
    if ($objects) {
      foreach ($objects as $object) {
        $pid = $object['PID'];
        $num_updated = db_update('collection_pids')
          ->fields([
            'indexed' => 1,
          ])
          ->condition('pid', $pid)
          ->execute();
      }
      drush_print("Batch $start completed.");
      $start = $start + $limit;
    }
  }
}

/**
 * One time function to remove OAI duplicated functions
 */
function drush_agile_lcp_remove_duplicates() {
  drush_print("Removing duplicates...");
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $duplicate_identifiers = agile_lcp_get_duplicates();
  $total = count($duplicate_identifiers);
  drush_print("$total found");
  foreach ($duplicate_identifiers as $identifier) {
    $pids_to_delete = [];
    $oai_only = TRUE;
    $query_processor = new IslandoraSolrQueryProcessor();
    $query_processor->solrQuery = "mods_recordInfo_recordIdentifier_s:$identifier";
    $query_processor->solrStart = 0;
    $query_processor->solrLimit = 100;
    $query_processor->solrParams = [
      'fl' => "PID RELS_EXT_hasModel_uri_s",
    ];
    $query_processor->executeQuery(FALSE);
    $solr_results = $query_processor->islandoraSolrResult;

    if ($solr_results['response']['objects']) {
      foreach ($solr_results['response']['objects'] as $object) {

        if ($object['solr_doc']['RELS_EXT_hasModel_uri_s'] == 'info:fedora/islandora:oai_importCModel') {
          $pids_to_delete[] = $object['PID'];
        }
        else {
          $oai_only = FALSE;
        }
      }
      if ($oai_only) {
        array_shift($pids_to_delete);
      }
      foreach ($pids_to_delete as $pid) {
        $obj = islandora_object_load($pid);
        drush_print("Deleting $obj->id");
        $obj->delete();
      }
    }
  }
}
