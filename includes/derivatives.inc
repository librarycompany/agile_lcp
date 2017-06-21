<?php

/**
 * @file
 * This file contains functions used to generate derivatives.
 */

/**
 * Creates the derivatives.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object in Fedora.
 *
 * @param bool $force
 *   Whether the derivative generation is being forced.
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
 * Gets all required datastreams from marc.
 *
 * @param xml $marc_xml
 *   MARC xml
 *
 * @return array
 *   Associative array of xmls to be ingested.
 */
function agile_lcp_get_streams_from_marc($marc_xml) {
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $mods_record = trim(agile_lcp_transform_with_saxonb('MARC21slim2MODS3-5_LCP.xsl', $marc_xml));
  $dc_record = trim(agile_lcp_transform_with_saxonb('marc_to_dc.xsl', $marc_xml));
  return array(
    'MODS' => $mods_record,
    'DC' => $dc_record,
  );
}
