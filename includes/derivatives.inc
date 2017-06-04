<?php

/**
 * @file
 * This file contains functions used to generate derivatives of implrted objects.
 */

/**
 * Creates the PROXY_MP3 derivative from the given audio file.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object in Fedora.
 *
 * @param bool $force
 *   Whether the derivative generation is being forced.
 *
 * @return array
 *   An array describing the outcome of the PROXY_MP3 creation.
 *
 * @see hook_islandora_derivative()
 */
function agile_lcp_create_derived_streams(AbstractObject $object, $force = FALSE) {
  if (!isset($object['MARC']) || (isset($object['MARC']) && $force === TRUE)) {
    if (!isset($object["MARC"])) {
      return array(
        'success' => FALSE,
        'messages' => array(
          array(
            'message' => t('No MARC datastream present for object @pid - MODS an DC creation were skipped.'),
            'message_sub' => array('@pid' => $object->id),
            'type' => 'watchdog',
            'severity' => WATCHDOG_ERROR,
          ),
        ),
      );
    }
    else {
      $marc = $object['MARC']->content;
      $streams = agile_lcp_get_streams_from_marc($object['MARC']->content);
      foreach ($streams as $dsid => $content) {
        if (isset($object[$dsid])) {
          $object[$dsid]->content = $content;
        }
        else {
          $type = ($dsid == 'DC') ? 'X' : 'M';
          $mime = ($dsid == 'DC') ? 'text/xml' : 'application/xml';
          $ds = $object->constructDatastream($dsid, 'M');
          $ds->setContentFromString($content, FALSE);
          $ds->label = $dsid;
          $ds->mimetype = $mime;
          $object->ingestDatastream($ds);
        }
      }
    }
  }
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
function agile_lcp_get_streams_from_marc($marc_xml) {
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $transform_args = array(
    'xsl' => drupal_get_path('module', 'agile_lcp') . '/xml/MARC21slim2MODS3-5_LCP.xsl',
    'input' => $marc_xml,
  );
  $mods_record = trim(agile_lcp_run_xsl_transform($transform_args));
  $transform_args = array(
    'xsl' => drupal_get_path('module', 'agile_lcp') . '/xml/marc_to_dc.xsl',
    'input' => $marc_xml,
  );
  $dc_record = trim(agile_lcp_run_xsl_transform($transform_args));
  return array(
    'MODS' => $mods_record,
    'DC' => $dc_record,
  );
}
