<?php
/**
 * @file
 * DB utilities.
 */

/**
 * Adds new token to database.
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
 * Get next unprocessed token id.
 *
 * @return int
 *   token id
 */
function agile_lcp_get_next_token_id() {
  $query = db_select('resumption_token', 'rt')
      ->condition('processed', 0);
  $query->addExpression('MIN(token_id)');
  $token_id = $query->execute()->fetchField();
  return $token_id;
}

/**
 * Get last unproccessed token.
 *
 * @return string
 *   token
 */
function agile_lcp_get_last_token() {
  $query = db_select('resumption_token', 'rt')
      ->condition('processed', 0)
      ->fields('rt', array('token')
  );
  $query->addExpression('MIN(token_id)');
  $token = $query->execute()->fetchField();
  return $token;
}

/**
 * Get token from id.
 *
 * @param int $token_id
 *   token_id
 * 
 * @return string
 *   token
 */
function agile_lcp_get_token_from_id($token_id) {
  $query = db_select('resumption_token', 'rt')
      ->fields('rt', array('token'))
      ->condition('token_id', $token_id);

  $token = $query->execute()->fetchField();
  return $token;
}

/**
 * Marks token as processed.
 *
 * @param string $token
 *   token being updated
 */
function agile_lcp_update_token($token) {
  $query = db_update('resumption_token')
          ->condition('token', $token)
          ->fields(array(
            'processed' => 1,
          ))->execute();
}

/**
 * Gets series of unprocessed redemption tokens.
 * 
 * @param int $limit
 *   number of ids to return
 *
 * @return array
 *   associative array of token ids and tokens
 */
function agile_lcp_get_unproccessed_tokens($limit = 1) {
  $query = db_select('resumption_token', 'rt')
      ->fields('rt', array('token_id', 'token'))
      ->condition('processed', 0)
      ->range(0, $limit);
  $result = $query->execute()
      ->fetchAll();
  $tokens = array();
  foreach ($result as $record) {
    $tokens[$record->token_id] = $record->token;
  }
  return $tokens;
}
