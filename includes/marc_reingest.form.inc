<?php

/**
 * @param array $form
 * @param array $form_state
 * @param \AbstractObject $object
 *
 * @return array
 */
function agile_reingest_marc_form(array $form, array $form_state, AbstractObject $object){
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $identifier = agile_lcp_get_identifier_from_marc($object);
  $form['identifier'] = array(
    '#title' => t('Update MarcXML from Aleph'),
    '#type' => 'textfield',
    '#description' => t('9 digit record identifier'),
    '#default_value' => $identifier,

  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Update MarcXML"),
  );
  return $form;
}

/**
 * @param array $form
 * @param array $form_state
 */
function agile_reingest_marc_form_submit(array $form, array $form_state) {
  module_load_include('inc', 'agile_lcp', 'includes/utilities');
  $object = islandora_object_load($form_state['build_info']['args'][0]->id);
  $identifier = $form_state['values']['identifier'];
  $identifier = str_pad($identifier, 9, 0,STR_PAD_LEFT);
  agile_update_marcXML($object, $identifier);
  drupal_goto("/islandora/object/{$object->id}/manage/datastreams");

}