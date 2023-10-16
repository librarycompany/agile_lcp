<?php
/**
 * @file
 * Utility functions.
 */

/**
 * Function build_full_dom.
 *
 * Description: returns dom object with all objects to be imported
 *
 * @param URL $url
 *   OAI url
 *
 * @return DomDocument
 *   DOMDocument
 */
function build_full_dom($url) {
  set_time_limit(0);
  $url_parts = explode('?', $url);
  $base_url = $url_parts[0] . "?verb=ListIdentifiers&resumptionToken=";
  $data = file_get_contents($url);
  if (!$data) {
    drupal_set_message(t("No results returned - please check url and login credentials", 'warning'));
    return;
  }
  $dom = new DOMDocument();
  $dom->loadXML($data);
  $resumption_node = $dom->getElementsByTagName('resumptionToken')->item(0);
  $token = $resumption_node->nodeValue;
  $tokens = 1;
  while ($token) {
    $new_url = $base_url . $token;
    $data = file_get_contents($new_url);
    $sub_dom = new DOMDocument();
    $sub_dom->loadXML($data);
    $resumption_node = $sub_dom->getElementsByTagName('resumptionToken')
      ->item(0);
    $token = $resumption_node->nodeValue;
    $tokens++;
  }
  return $tokens;
}

/**
 * Gets all required datastreams from identifier.
 *
 * @param string $identifier
 *   OAI Identifier
 *
 * @return array
 *   Associative array of xmls to be ingested.
 */

function agile_get_streams_from_entry($identifier) {
  // Remove OAI qualifier from MARC identifier.
  $identifier = preg_replace('/oai:librarycompany.org:/', '', $identifier);
  module_load_include('inc', 'islandora_marcxml', 'includes/utilities');
  $url = "https://librarycompany.kohacatalog.com/cgi-bin/koha/oai.pl?verb=GetRecord&identifier=oai:librarycompany.org:$identifier&metadataPrefix=marc21";
  $marc_NS = "http://www.loc.gov/MARC21/slim";
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $deleted = $dom->getElementsByTagName('header')->item(0)->getAttribute('status') == 'deleted';
  if ($deleted) {
    return NULL;
  }
  $record_node = $dom->getElementsByTagNameNS($marc_NS, 'record')->item(0);
  $marc_record = $dom->saveXML($record_node);
  //$mods_record = trim(agile_lcp_transform_with_saxonb('MARC21slim2MODS3-5_LCP.xsl', $marc_record));
  //$dc_record = trim(agile_lcp_transform_with_saxonb('marc_to_dc.xsl', $marc_record));
  return [
    //'MODS' => $mods_record,
    //'DC' => $dc_record,
    'MARC' => $marc_record,
  ];

}

/**
 * Run an XSLT, and return the results.
 *
 * @param array $info
 *   An associative array of parameters, containing:
 *   - input: The input XML in a string.
 *   - xsl: The path to an XSLT file.
 *
 * @return string
 *   The transformed XML, as a string.
 */
function agile_lcp_run_xsl_transform($info) {
  $xsl = new DOMDocument();
  $xsl->load($info['xsl']);
  $input = new DOMDocument();
  $input->loadXML($info['input']);
  $processor = new XSLTProcessor();
  $processor->importStylesheet($xsl);
  return @$processor->transformToXML($input);
}

/**
 * Gets pids from OAI identifier.
 *
 * @param string $identifier
 *   OAI identifier
 *
 * @return array
 *   Array of pids
 */
function agile_lcp_get_pids_from_identifier($identifier, $oai_only = FALSE) {
  // Remove OAI qualifier from MARC identifier.
  $identifier = preg_replace('/oai:librarycompany.org:/', '', $identifier);
  $pids = [];
  if (!is_numeric($identifier)) {
    drush_print(" Identifier $identifier is not numeric.");
    return $pids;
  }
  $qualifier = $oai_only ? '+RELS_EXT_hasModel_uri_s:"info:fedora/islandora:oai_importCModel"' : '';
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = "+mods_recordInfo_recordIdentifier_s:$identifier $qualifier";
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 50;

  $query_processor->solrParams = [
    'fl' => "PID",
  ];
  $query_processor->executeQuery(FALSE);
  $solr_results = $query_processor->islandoraSolrResult;

  if ($solr_results['response']['objects']) {
    foreach ($solr_results['response']['objects'] as $object) {
      $pid = $object['PID'];
      drush_print(" Found existing object $pid.");
      $pids[] = $object['PID'];
    }
  } else {
    drush_print(" No matching objects found.");
  }
  return $pids;
}

/**
 * Gets pids where more than one Islandora object has same OAI identifier.
 *
 * @return array
 *   PIDS
 */
function agile_lcp_get_duplicates() {
  $query = 'mods_recordInfo_recordIdentifier_s:*';
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('*:*');
  $qp->solrParams['fq'] = $query;
  $qp->solrParams['facet'] = 'true';
  $qp->solrParams['facet.limit'] = 500;
  $qp->solrParams['facet.mincount'] = 2;
  $qp->solrParams['facet.field'] = [
    'mods_recordInfo_recordIdentifier_s',
    'RELS_EXT_hasModel_uri_s',
  ];
  $qp->solrParams['fl'] = 'PID mods_recordInfo_recordIdentifier_s';
  $qp->executeQuery(FALSE);
  $identifiers = array_keys($qp->islandoraSolrResult['facet_counts']['facet_fields']['mods_recordInfo_recordIdentifier_s']);
  return $identifiers;
}

/**
 * Helper to transform XML using the named XSLT with saxonb.
 *
 * Taken from DGI's lcpdora module with minor changes.
 *
 * @param string $xsl_name
 *   Name of the XSL file to be used, relative to this module's "xml"
 *   directory.
 * @param string $doc
 *   The XML to convert.
 * @param array $params
 *   An optional associative array of XSLT parameters mapping parameter names
 *   to their values.
 *
 * @return string
 *   The transformed xml.
 */
function agile_lcp_transform_with_saxonb($xsl_name, $doc, $params = []) {
  $module_path = drupal_get_path('module', 'agile_lcp');
  $xsl_path = format_string('!path/xml/!xsl', [
    '!path' => $module_path,
    '!xsl' => $xsl_name,
  ]);
  $pipes = [];
  $equal_join_and_escape = function ($key, $value) {
    return escapeshellarg("$key=$value");
  };
  $escaped = array_map($equal_join_and_escape, array_keys($params), $params);
  $param_string = implode(' ', $escaped);
  $process = proc_open("saxonb-xslt -versionmsg:off - $xsl_path $param_string",
    [
      0 => [
        'pipe',
        'r',
      ],
      1 => [
        'pipe',
        'w',
      ],
      2 => [
        'file',
        '/dev/null',
        'a',
      ],
    ],
    $pipes
  );
  fwrite($pipes[0], $doc);
  fclose($pipes[0]);
  $saxonb_transformed = stream_get_contents($pipes[1]);
  fclose($pipes[1]);
  proc_close($process);
  return $saxonb_transformed;
}


/**
 * Get all pids from islandora:imported_oai collection.
 */
function agile_lcp_get_collection_pids() {
  $limit = 10000;
  $tuque = new IslandoraTuque();
  for ($i = 0; $i < 4; $i++) {
    $offset = $i * $limit;
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
    foreach ($results as $result) {
      $nid = db_insert('collection_pids')
        ->fields([
          'pid' => $result['pid']['value'],
          'indexed' => 0,
        ])
        ->execute();
    }
  }
}


/**
 * @param \AbstractObject $Object
 */
function agile_update_marcXML($object, $identifier) {
  $streams = agile_get_streams_from_entry($identifier);
  // Allow MARC content for objects without existing MarcXML.
  foreach ($streams as $key => $content) {
    if ($key == "MARC" && !$object['MARC']) {
      $ds = $object->constructDatastream('MARC', 'M');
      $ds->label = 'MARC';
      $ds->mimetype = 'text/xml';
      $ds->content = $content;
      $object->ingestDatastream($ds);
      continue;
    }
    $object[$key]->content = $content;
  }
  drupal_set_message(t('Updated records'));
}

/**
 * @param \AbstractObject $object
 *
 * @return string
 */
function agile_lcp_get_identifier_from_marc(AbstractObject $object) {
  if (!$object['MARC']) {
    return NULL;
  }
  $marc = $object['MARC']->content;
  $dom = new DOMDocument();
  $dom->loadXML($marc);
  $xpath = new DOMXPath($dom);
  $xpath->registerNamespace('marc', "http://www.loc.gov/MARC21/slim");

  // new identifier from Koha is in 999$c
  $query = "//marc:datafield[@tag='999']/marc:subfield[@code='c']";
  $identifiers = $xpath->query($query);
  if ($identifiers->length) {
    return $identifiers->item(0)->nodeValue;
  }
  else {

    // if no new identifier, we've got an old MARC, so fetch it from the 001
    $query = "//marc:controlfield[@tag = '001']";
    $identifiers = $xpath->query($query);
    if ($identifiers->length) {
      return $identifiers->item(0)->nodeValue;
    }
  }

  return NULL;
}


/**
 * @param string $identifier
 *
 * @return array|null
 */
function agile_lcp_get_duplicates_from_identifier(string $identifier) {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = "mods_recordInfo_recordIdentifier_s:$identifier";
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = [
    'fl' => "PID RELS_EXT_hasModel_uri_s",
  ];
  $query_processor->executeQuery(FALSE);
  $solr_results = $query_processor->islandoraSolrResult;
  $pids = [];
  $pids_to_delete = [];
  if ($solr_results['response']['objects']) {
    foreach ($solr_results['response']['objects'] as $object) {
      $content_model = $object['solr_doc']['RELS_EXT_hasModel_uri_s'];
      if ($content_model == 'info:fedora/islandora:oai_importCModel') {
        $pids_to_delete[] = $object['PID'];
      }
      $pids[] = $object['PID'];
    }
  }
  return count($pids) > count($pids_to_delete) ? $pids_to_delete : NULL;

}


/**
 * @param $identifier
 *
 * @return array
 */
function agile_lcp_get_oai_objects_from_identifier($identifier = NULL) {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = "+mods_recordInfo_recordIdentifier_s:$identifier +RELS_EXT_hasModel_uri_s:\"info:fedora/islandora:oai_importCModel\" ";
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = [
    'fl' => "PID RELS_EXT_hasModel_uri_s",
  ];
  $query_processor->executeQuery(FALSE);
  $oai_pids = [];
  $solr_results = $query_processor->islandoraSolrResult;
  foreach ($solr_results['response']['objects'] as $object) {
    $oai_pids[] = $object['PID'];
  }
  return $oai_pids;
}

function agile_lcp_find_empty_objects() {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = '-mods_titleInfo_title_s:* +fedora_datastreams_ms:MODS +RELS_EXT_hasModel_uri_s:"info:fedora/islandora:oai_importCModel"';
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = [
    'fl' => "PID",
  ];
  $query_processor->executeQuery(FALSE);
  $pids = [];
  $solr_results = $query_processor->islandoraSolrResult;
  foreach ($solr_results['response']['objects'] as $object) {
    $pids[] = $object['PID'];
  }

  return $pids;
}
