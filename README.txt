
DESCRIPTION
===========
The IP Geolocation module provides ready-to-use views, blocks and maps of
your site's visitor locations, based on their IP addresses.

IP Geolocation is therefore different from modules like Location and GMap, which
are about allowing users to enter lat/long coordinates or postal address
information for content and user profiles and then mapping those.

IP Geolocation takes advantage of its own simple database layer for efficient
customisable reporting of your visitors' locations, both past and present.
The reporting capability (through Views) raises IP Geolocation above other
modules operating in this space. For instance, using IP Geolocation you can
display (through tables and/or maps) where in the world your visitors came from
and how many visited during what period of time.
Globe coordinates (i.e. lat/long), maps and postal addresses may be displayed,
often down to street granularity. All this going back to the day your site was
launched!

IP Geolocation takes advantage of existing tried-and-tested contributed modules
to pull in visitor geolocation data, especially historic data (for present and
future visitor data we recommended to use IP Geolocation's built-in collection
method). Just enable your favorite module and you're away. The following modules
are supported as IP-to-lat/long data sources:
o Smart IP
o GeoIP API
o any module that implements hook_get_ip_geolocation_alter($location). See the
  programmer's section below for details.

You may enable more than one of the above at the same time, so that if one
module is unable to provide all the geolocation details for a certain IP 
address, another one may add to it.

You do not need to register for any API keys, except when you use Smart IP to
back-fill your visitor history using the web service provided by 
www.ipinfodb.com. The API key required on the Smart IP configuration page is
free and is sent to you immediately by return email after applying at
http://ipinfodb.com/register.php

So what's in the box?
o a block displaying a map with marked locations of your most recent, say, 20
  visitors; each marker when clicked reveals in a balloon that visitor's IP
  address, full postal address (including suburb, street and number), number of
  page visits and date & times of the most recent visit
o for sorting, reporting and exporting purposes: a number of configurable views
  displaying ordered lists of for instance ip addresses that visitied your site, 
  optionally grouped by country, with for each detailed address information, a
  small map (one per IP address), page visit counts, first and last visits etc;
  naturally being views you can add, remove and rearrange columns as you wish
o for programmers: an API to retrieve lat/long and address information and to
  generate maps; also a hook, hook_get_ip_geolocation_alter(), to insert your
  own geolocation data retrieval function, if you so wish -- see the FOR
  PROGRAMMERS section below.

INSTALLATION & CONFIGURATION
============================
Pick your geolocation data source module from the ones listed in the 
introduction and follow the instructions for that module. Smart IP does not
require you to install any additional files, but you may have to request a free
key from the data provider, which will be sent to you by return email.
Apart from that you really don't have to do much at all to configure IP 
Geolocation. It figures out itself which geolocation data module(s) it is
dealing with. Visit the Configuration >> IP Geolocation page and you'll find
that the configuration options available are very much self-explanatory.

Further info may be found at the module's project page and issue queue, see
http://drupal.org/project/ip_geoloc

FOR PROGRAMMERS
===============
Hooking your own gelocation data provider into IP Geolocation is very simple.
In your module, let's call it MYMODULE, all you have to do is flesh out the
following function.
<?php
/*
 *  Implements hook_get_ip_geolocation_alter().
 */
function MYMODULE_get_ip_geolocation_alter(&$location) {

  if (empty($location['ip_address'])) {
    return;
  }
  // ... your code here to retrieve geolocation data ...

  // Then fill out some or all of the location fields that IP Geolocation knows
  // how to store.
  $location['latitude'] =  ....;
  $location['longitude'] = ....;
  $location['country'] = ....;
  $location['country_code'] = ....;
  $location['region'] = ....;
  $location['region_code'] = ....;
  $location['city'] = ... ;
  $location['locality'] = ....; // eg suburb
  $location['route'] = ....;     // eg street
  $location['street_number'] = ....; 
  $location['postal_code'] = ....; // eg ZIP
  $location['administrative_area_level_1'] = ....; // eg state or province
  $location['formatted_address'] = ....; // address as a human-readible string
}
?>
That's all!
Note that when IP Geolocation calls this function the $location object may be
partially fleshed out. If $location['ip_address'] is empty, this means that
IP Geolocation is still waiting for more details to arrive from the Google
reverse-geocoding AJAX call. If $location['ip_address'] is not empty, then IP
Geolocation does not expect any further details and will store the $location
with your modifications (if any) on the IP Geolocation database. You must set
$location['formatted_address'] in order for the location to be stored.

RESTRICTIONS IMPOSED BY GOOGLE
==============================
Taken from http://code.google.com/apis/maps/documentation/geocoding :

"Use of the Google Geocoding API is subject to a query limit of 2500 geolocation
requests per day. (Users of Google Maps API Premier may perform up to 100,000
requests per day.) This limit is enforced to prevent abuse and/or repurposing of
the Geocoding API, and this limit may be changed in the future without notice.
Additionally, we enforce a request rate limit to prevent abuse of the service.
If you exceed the 24-hour limit or otherwise abuse the service, the Geocoding 
API may stop working for you temporarily. If you continue to exceed this limit,
your access to the Geocoding API may be blocked.

Note: the Geocoding API may only be used in conjunction with a Google map;
geocoding results without displaying them on a map is prohibited. For complete
details on allowed usage, consult the Maps API Terms of Service License
Restrictions."

AUTHOR
======
Rik de Boer, Melbourne, Australia.
