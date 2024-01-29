#!/usr/bin/perl
# Matija Nalis <mnalis-perl@voyager.hr> GPLv3+ started 2024-01-29
#
# use in pipeline to extract only specific users from OSM history planet dump, like so:
# curl https://planet.openstreetmap.org/planet/full-history/history-latest.osm.bz2 | pbzip2 -dc | ./osm-extract-uid.pl '1234'
#
# accepts regex, so '1234|5678' will extract both UIDs
#

use warnings;
use strict;
use autodie qw/:all/;

while (<STDIN>)
{
}