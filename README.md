## Agile LCP Customization Module

The Agile LCP module allows OAI harvesting and updating, and alters object display to show links to the original catalog.

The only step necessary to see the link in items harvested from the catalog is to ensure that the Solr index is configured to display the solr mods_recordInfo_recordIdentifier_s field.

### Building the collection
Building the collection is a 2 step process.  First resumption tokens are harvested from the catalog and put in a database, then in a series of batch processes a collection of unique identifiers are harvested from the OAI source and the marc record from this entry is harvested, converted to both MODS and DC and an Islandora fedora object is built with all three datastreams and put in the default collection.  Either of these steps can be done from either the web interface or the command line using drush.  Drush is the preferred method for long running processes, but both methods work well.

### Web interface.
To prep the database navigate to http://digital.librarycompany.org/agile/prep/oai and click the Prepare Harvest button.  Harvesting will take a few minutes.

To harvest records navigate to http://digital.librarycompany.org/agile/harvest/oai, indicate how many tokens you’d like to process and click Import via OAI.  Each token takes about 45 seconds to complete under ideal circumstances.  Completion will take longer while the server is under heavy load.  It will take about 120 hours of batch time to complete the entire job.

### Drush.
Once shh’ed into the server navigate to your Drupal installation – normally at /var/www/drupal7/.

To initialize the harvest type drush OAI_Prep at the command line.  You will see a progressive dotted line indicating the rate t which tokens are being recorded.  You’ll receive a printed message when the harvesting is complete. If you wish to use anything other than the default url type drush OAI_Prep --url=your_url;

To harvest tokens type drush OAI to harvest a single token, or drush OAI limit=<any reasonable number up to about 700>.  On the first run only, type drush OAI new=TRUE – this will harvest the first group of records, the only records harvestable without a resumption token.

### Rebuilding
To rebuild the entire index navigate to http://digital.librarycompany.org/islandora/object/islandora%3Aimported_oai/manage/properties and delete entire contents.  This will take some time – there are over 200,000 objects there.  The empty directory can be restored by navigating to http://digital.librarycompany.org/admin/islandora/solution_pack_config/solution_packs and rebuilding the Imported From OAI Collection.

Proceed as per directions above.

[in progress]
