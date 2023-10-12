<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function agile_lcp_admin_settings_form(array $form, array &$form_state) {
  $search_node = variable_get('agile_lcp_search_node_id', '31');
  $dropbox_api = variable_get('lcpdora_dropbox_token', '');
  if ($search_node) {
    $form['about'] = array(
      '#type' => 'markup',
      '#markup' => l(t('Go to node'), "/node/$search_node/edit"),
    );
  }
  $form['agile_lcp_search_node_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Node ID of search block'),
    '#description' => t("The Drupal node containing the page code for search"),
    '#default_value' => $search_node,
    '#size' => 5,
  );
  $form['lcpdora_dropbox_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Dropbox API Access Token'),
    '#description' => t("Access token for Dropbox account containing the object bags"),
    '#default_value' => $dropbox_api,
    '#size' => 100,
  );
  return system_settings_form($form);
}
