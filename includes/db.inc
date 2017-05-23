<?php

/**
 * Adds new token to database
 * 
 * @param string $token
 *   token to add.
 */
function agile_lcp_add_token($token) {
  $nid = db_insert('resumption_token')
      ->fields(array(
        'token' => $token,
        'processed' => 0,
      ))
      ->execute();
}

/**
 * Get most unprocessed token id
 *
 * @return integer
 *   token id
 */
function agile_lcp_get_last_token_id() {
  $query = db_select('resumption_token', 'rt')
      ->condition('processed', 0);
  $query->addExpression('MAX(token_id)');
  $token_id = $query->execute()->fetchField();
  return $token_id;
}

/**
 * Get last unproccessed token
 *
 * @return string
 *   token
 */
function agile_lcp_get_last_token() {
  $query = db_select('resumption_token', 'rt')
      ->condition('processed', 0)
      ->fields('rt', array('token')
  );
  $query->addExpression('MAX(token_id)');
  $token = $query->execute()->fetchField();
  return $token;
}

/**
 * Get token from id.
 *
 * @param integer $token_id
 *   token_id
 * 
 * @return string
 *   token
 */
function agile_lcp_get_token_from_id($token_id) {
  $query = db_select('resumption_token', 'rt')
      ->fields('rt', array('token'))
      ->condition('token_id', 0);

  $token = $query->execute()->fetchField();
  return $token;
}

/**
 * Marks token as processed.
 *
 * @param string $token
 */
function agile_lcp_update_token($token) {
  $query = db_update('resumption_token')
          ->condition('token', $token)
          ->fields(array(
            'processed' => 1,
          ))->execute();
}
