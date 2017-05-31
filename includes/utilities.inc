<?php

/**
 * Function build_full_dom
 *
 * Description: returns dom object with all objects to be imported
 * @param URL $url
 * @return DomDocument
 */
function build_full_dom($url) {
  $identifiers = array();
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
  $tokens = 1;
  $new_identifiers = $dom->getElementsByTagName('identifier');
  foreach ($new_identifiers as $new_identifier) {
    //$identifiers[] = $new_identifier->nodeValue;
  }
  while ($token) {
    $new_url = $base_url . $token;
    $data = file_get_contents($new_url);
    $sub_dom = new DOMDocument();
    $sub_dom->loadXML($data);
    $new_identifiers = $dom->getElementsByTagName('identifier');
    foreach ($new_identifiers as $new_identifier) {
      //$identifiers[] = $new_identifier->nodeValue;
    }
    $resumption_node = $sub_dom->getElementsByTagName('resumptionToken')->item(0);
    $token = $resumption_node->nodeValue;
    $tokens++;
  }
  return $tokens;
}

function get_streams_from_entry($identifier) {
  module_load_include('inc', 'islandora_marcxml', 'includes/utilities');
  $url = "http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=GetRecord&identifier=$identifier&metadataPrefix=marc21";
  $marcNS = "http://www.loc.gov/MARC21/slim";
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $record_node = $dom->getElementsByTagNameNS($marcNS, 'record')->item(0);
  $marc_record = $dom->saveXML($record_node);
  $transform_args = array(
    'xsl' => drupal_get_path('module', 'agile_lcp') . '/xml/MARC21slim2MODS3-5_LCP.xsl',
    'input' => $marc_record,
  );
  $mods_record = trim(agile_lcp_run_xsl_transform($transform_args));
  $transform_args = array(
    'xsl' => drupal_get_path('module', 'agile_lcp') . '/xml/marc_to_dc.xsl',
    'input' => $marc_record,
  );
  $dc_record = trim(agile_lcp_run_xsl_transform($transform_args));
  return array(
    'MODS' => $mods_record,
    'DC' => $dc_record,
    'MARC' => $marc_record,
  );
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
 * Gets pids from OAI identifier
 *
 * @param string $identifier
 *   OAI identifier
 *
 * @return array
 *   Array of pids
 */
function agile_lcp_get_pids_from_identifier($identifier) {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = "mods_recordInfo_recordIdentifier_s:identifier";
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 10;
  $query_processor->solrParams = array(
    'fl' => "PID",
  );
  $query_processor->executeQuery(FALSE);
  $solr_results = $query_processor->islandoraSolrResult;
  $pids = array();
  if ($solr_results['response']['objects']) {
    foreach ($solr_results['response']['objects'] as $object) {
      $pids[] = $object['PID'];
    }
  }
  return $pids;
}

/**
 * Gets pids where more than one Islandora oject has same OAI identifier
 * 
 * @return array
 *   PIDS
 */
function agile_lcp_get_duplicates() {
  $query = "mods_recordInfo_recordIdentifier_s:*";
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery('*:*');
  $qp->solrParams['fq'] = $query;
  $qp->solrParams['facet'] = 'true';
  $qp->solrParams['facet.limit'] = 200;
  $qp->solrParams['facet.mincount'] = 2;
  $qp->solrParams['facet.field'] = array('mods_recordInfo_recordIdentifier_s');
  $qp->solrParams['fl'] = 'PID';
  $qp->executeQuery(FALSE);
  return $qp->islandoraSolrResult['facet_counts']['facet_fields']['mods_recordInfo_recordIdentifier_s'];
}

