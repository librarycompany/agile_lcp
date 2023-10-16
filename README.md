## Agile LCP Customization Module

The Agile LCP module allows OAI harvesting and updating, and alters object display to show links to the original catalog.

The only step necessary to see the link in items harvested from the catalog is to ensure that the Solr index is configured to display the solr mods_recordInfo_recordIdentifier_s field.

### Building the collection
Building the collection is a two-step process.  First, resumption tokens are harvested from the catalog and put in a database, then in a series of batch processes a collection of unique identifiers are harvested from the OAI source and the marc record from this entry is harvested, converted to both MODS and DC and an Islandora fedora object is built with all three datastreams and put in the default collection.  

Please note that the features to ingest new MARC records and to have a separate OAI collection to use as a unified discovery layer has been disabled per LCP request. The code is commented out and can be re-implemented if desired, but it was written for Aleph and needs to be validated against Koha.

### Drush.
**Standard Workflow**

1. ssh into the server
2. Sudo to the root user: `sudo bash`
3. Navigate to your Drupal installation: `cd /var/www/drupal7/`
4. To initialize the harvest type `drush -u 1 OAI_Prep` at the command line.  You will see a progressive dotted line indicating the rate at which resumption tokens are being recorded.  You'll receive a printed message when the harvesting is complete. Each token contains about 50 records.
5. To harvest tokens type `drush -u 1 OAI` to harvest a single token, or `drush -u 1 OAI --limit=100` to harvest a larger group. Limit can be any reasonable number up to about 500. For update harvests, there may only be a couple tokens, so `limit=100` will cover it.

**Additional Functionality**

* By default, the harvest prep will continue from the last harvest date. All other parameters are ignored. If you want to start with a new date range, or from scratch, you have to specify `drush -u 1 OAI_Prep --reset=1` along with the other parameters.

* Results may be limited to only those records with datestamps within a range, by using `from` or `to` as optional parameters. For example, `drush -u 1 OAI_Prep --from=1999-12-31 –-to=2016-12-31`

It is not at all unusual for a batch job to stop before completion.  This may be caused by network glitches, or system overuse.  Batches can be restarted at any time with no loss of data.

### Web interface.
**DO NOT USE THE WEB INTERFACE. IT HAS NOT BEEN UPDATED TO MATCH THE DRUSH FUNCTIONALITY. USE DRUSH.**

To prep the database navigate to http://digital.librarycompany.org/agile/prep/oai and click the Prepare Harvest button.  The default url brings back all records from the default site.  Datestamps, if entered, will be limit the search results to those records lying between and including the datestamps. Both ‘from’ and ‘to’ are optional.  Harvesting will take a few minutes, so please be patient.

To harvest records navigate to http://digital.librarycompany.org/agile/harvest/oai, indicate how many tokens you’d like to process and click Import via OAI.  Each token takes about 45 seconds to complete under ideal circumstances.  Completion will take longer while the server is under heavy load.  It will take about 120 hours of batch time to complete the entire job.

### Rebuilding
To rebuild the entire index navigate to http://digital.librarycompany.org/islandora/object/islandora%3Aimported_oai/manage/properties and delete entire contents.  This will take some time – there are over 200,000 objects there.  The empty directory can be restored by navigating to http://digital.librarycompany.org/admin/islandora/solution_pack_config/solution_packs and rebuilding the Imported From OAI Collection.

Proceed as per directions above.

### Licence
Licenced under a GNU General Public License v3.0. 
https://www.gnu.org/licenses/gpl-3.0.en.html
