<?php

function agile_lcp_harvest_form($form, &$form_state) {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $token = agile_lcp_get_last_token_id();
  if (!$token) {
    $token = t('No tokens processed yet');
  }
  else {
    $token = t('Next token ID to process is @token', array('@token' => $token));
  }
  $form = array();
  $form['info'] = array(
    '#type' => 'markup',
    '#markup' => $token,
  );
  $form['process'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#title' => 'Import from DOA',
    '#description' => t('Number of tokens to process'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Import via DOA"),
  );
  return $form;
}


function agile_cp_create_objects($identifiers) {
  foreach ($identifiers as $identifier) {
    agile_lcp_build_object($identifier);
  }
}

function agile_lcp_build_object($identifier) {
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $streams = get_streams_from_entry($identifier);
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $object = $repository->constructObject('islandora');
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:doa_importCModel');
  $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:imported_doa');
  $dom = new DOMDocument();
  $dom->loadXML($streams['MODS']);
  $title = $dom->getElementsByTagName('title')->item(0)->nodeValue;
  $object->label = $dom->getElementsByTagName('title')->item(0)->nodeValue;
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


function agile_lcp_harvest_form_submit($form, &$form_state) {

  $iterations = $form_state['values']['process'];
  $batch = array(
    'title' => t('Importing records'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'agile_lcp') . '/includes/harvest_form.inc',
    'progress_message' => t('@current of @total operations completed.'),
  );

  for ($i = 0; $i < $iterations; $i++) {

    $batch['operations'][] = array('agile_lcp_import_doa', array());
  }
  batch_set($batch);
  batch_process();
}

function agile_lcp_import_doa() {
  module_load_include('inc', 'agile_lcp', 'includes/db');
  $token = agile_lcp_get_last_token();
  if ($token) {
    $url = "http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&resumptionToken=$token";
  }
  else {
    $url = 'http://pacscl.hosted.exlibrisgroup.com:48992/OAI?verb=ListIdentifiers&set=OAI-SET&metadataPrefix=marc21';
  }
  $input = file_get_contents($url);
  $dom = new DOMDocument();
  $dom->loadXML($input);
  $resumption_node = $dom->getElementsByTagName('resumptionToken')->item(0);
  $resumption_token = $resumption_node->nodeValue;
  if ($resumption_token) {
    agile_lcp_add_token($resumption_token);
  }

  $new_identifiers = $dom->getElementsByTagName('identifier');
  foreach ($new_identifiers as $new_identifier) {
    $identifiers[] = $new_identifier->nodeValue;
  }
  agile_cp_create_objects($identifiers);
  agile_lcp_update_token($token);
}
