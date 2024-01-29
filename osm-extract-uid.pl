#!/usr/bin/perl
# Matija Nalis <mnalis-perl@voyager.hr> GPLv3+ started 2024-01-29
#
# use in pipeline to extract only specific users from OSM history planet dump, like so:
# curl https://planet.openstreetmap.org/planet/full-history/history-latest.osm.bz2 | pbzip2 -dc | ./osm-extract-uid.pl '1234'
#
# accepts regex, so '1234|5678' will extract both UIDs
#
# FIXME: one should never parse XML like this, it will break. Use SAX or something to avoid loading DOM, if not too slow

use warnings;
use strict;
use autodie qw/:all/;

my $DEBUG = 1;

my $interesting_uid = $ARGV[0];
die "no UID specified" unless $interesting_uid;

my $active = 0;
while (<STDIN>)
{
    #$DEBUG > 9 && print STDERR "line: $_";
    
    if ($active) {
        print;
        $active = 0 if /^\s*<\/changeset>\s*$/;    # changeset tag closed, do not print anymore
    }
    
    next unless /<changeset /;                    # fast path skip only
    next unless / uid="${interesting_uid}" /;     # fast path skip only

    $DEBUG > 7 && print "interesing uid $interesting_uid starts here:\n";
    
    if (/^\s*<changeset .*uid="${interesting_uid}"/) {
        print;  # always print start of changeset
        next if /\/>\s*$/; # if changeset XML tag is auto-closed in same line, no further action needed
        die "unparsable line $_" unless />\s*$/;
        $active = 1;    # print everything until the end of the changeset
    }
}