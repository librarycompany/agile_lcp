<?php

/**
 * @file
 * Class override for LCP display
 */

/**
 * Extension of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsLCP extends IslandoraSolrResults {

  /**
   * Overrides displayResults.
   */
  public function displayResults($islandora_solr_query) {
    $this->islandoraSolrQueryProcessor = $islandora_solr_query;
    // Set variables to collect returned data.
    $results = NULL;
    $secondary_profiles = NULL;
    $elements = array();
    // Set breadcrumbs.
    $this->setBreadcrumbs($islandora_solr_query);
    // Raw solr results.
    $islandora_solr_result = $this->islandoraSolrQueryProcessor->islandoraSolrResult;
    $page_total = count($islandora_solr_result['response']['objects']);
    $max = $islandora_solr_query->solrLimit;

    // Solr results count.
    // Total Solr results.
    $elements['solr_total'] = (int) $islandora_solr_result['response']['numFound'];

    // Solr start.
    // To display: $islandora_solr_query->solrStart + ($total > 0 ? 1 : 0).
    $elements['solr_start'] = $islandora_solr_query->solrStart;

    // Solr results end.
    $end = min(($islandora_solr_query->solrLimit + $elements['solr_start']), $elements['solr_total'], $page_total);
    $elements['solr_end'] = $end;

    // Pager.
    islandora_solr_pager_init($elements['solr_total'], $islandora_solr_query->solrLimit);
    $elements['solr_pager'] = theme('pager', array(
      'tags' => NULL,
      'element' => 0,
      'parameters' => NULL,
      'quantity' => 5,
    ));

    // Debug (will be removed).
    $elements['solr_debug'] = '';
    if (variable_get('islandora_solr_debug_mode', 0)) {
      $elements['solr_debug'] = $this->printDebugOutput($islandora_solr_result);
    }

    // Rendered secondary display profiles.
    $secondary_profiles = $this->addSecondaries($islandora_solr_query);

    // Rendered results.
    $results = $this->printResults($islandora_solr_result);
    if ($elements['solr_total'] < $max) {
      $elements['solr_total'] = $page_total;
    }

    return theme('islandora_solr_wrapper', array(
      'results' => $results,
      'secondary_profiles' => $secondary_profiles,
      'elements' => $elements,
    ));
  }

}
