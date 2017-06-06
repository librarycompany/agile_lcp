## Agile LCP Customization Module

The Agile LCP module allows OAI harvesting and updating, and alters object display to show links to the original catalog.

The only step necessary to see the link in items harvested from the catalog is to ensure that the Solr index is configured to display the solr mods_recordInfo_recordIdentifier_s field.

### Building the collection
Building the collection is a two-step process.  First, resumption tokens are harvested from the catalog and put in a database, then in a series of batch processes a collection of unique identifiers are harvested from the OAI source and the marc record from this entry is harvested, converted to both MODS and DC and an Islandora fedora object is built with all three datastreams and put in the default collection.  Either of these steps can be done from either the web interface or the command line using drush.  Drush is the preferred method for long running processes, but both methods work well.

### Web interface.
To prep the database navigate to http://digital.librarycompany.org/agile/prep/oai and click the Prepare Harvest button.  The default url brings back all records from the default site.  Datestamps, if entered, will be limit the search results to those records lying between and including the datestamps. Both ‘from’ and ‘to’ are optional.  Harvesting will take a few minutes, so please be patient.

To harvest records navigate to http://digital.librarycompany.org/agile/harvest/oai, indicate how many tokens you’d like to process and click Import via OAI.  Each token takes about 45 seconds to complete under ideal circumstances.  Completion will take longer while the server is under heavy load.  It will take about 120 hours of batch time to complete the entire job.

### Drush.
Once you ssh into the server, navigate to your Drupal installation, which is normally at /var/www/drupal7/.

To initialize the harvest type drush OAI_Prep at the command line.  You will see a progressive dotted line indicating the rate at which tokens are being recorded.  You’ll receive a printed message when the harvesting is complete. If you wish to use anything other than the default url type drush OAI_Prep --url=your_url;  Results may be limited to only those records with datestamps within a range.  Either ‘from’ or ‘to’ may be used as optional parameters.  Ie OAI_Prep --from=1999-12-31 –to=2016-12-31

To harvest tokens type drush OAI to harvest a single token, or drush OAI limit=<any reasonable number up to about 700> to harvest a larger group.  On the first run only, type drush OAI new=TRUE 
This will harvest the first group of records, the only records harvestable without a resumption token.  

It is not at all unusual for a batch job to stop before completion.  This may be caused by network glitches, or system overuse.  The batch can be restarted at any time with no loss of data.

### Rebuilding
To rebuild the entire index navigate to http://digital.librarycompany.org/islandora/object/islandora%3Aimported_oai/manage/properties and delete entire contents.  This will take some time – there are over 200,000 objects there.  The empty directory can be restored by navigating to http://digital.librarycompany.org/admin/islandora/solution_pack_config/solution_packs and rebuilding the Imported From OAI Collection.

Proceed as per directions above.

### Licence
Licenced under a GNU General Public License v3.0. 
https://www.gnu.org/licenses/gpl-3.0.en.html
