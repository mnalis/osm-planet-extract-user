#!/usr/bin/perl
# Matija Nalis <mnalis-perl@voyager.hr> GPLv3+ started 2024-01-29
#
# use in pipeline to extract only specific users from OSM history planet dump, like so:
# curl -sL https://planet.openstreetmap.org/planet/full-history/history-latest.osm.bz2 | pbzip2 -dc | ./osm-extract-uid.pl '1234'
#
# accepts regex, so '1234|5678' will extract both UIDs
#
# FIXME: one should never parse XML like this, it will break. Use SAX or something to avoid loading DOM, if not too slow

use warnings;
use strict;
use autodie qw/:all/;

my $DEBUG = $ENV{DEBUG} || 0;

my $interesting_uid = $ARGV[0];
die "no UID specified" unless $interesting_uid;
if ($interesting_uid =~ /^\d.*\|/) {
    # auto-fix interesting uid if it contains "|" but does not start with "("
    $interesting_uid = '(' . $interesting_uid . ')';
}

$DEBUG > 0 && print STDERR "Searching for UIDs: $interesting_uid\n";

my $active = 0;
while (<STDIN>)
{
    # $DEBUG > 9 && print STDERR "DBG: $_";
    
    if ($active) {
        print;
        $active = 0 if /^\s*<\/${active}>\s*$/;   # changeset/node/way/relation tag has just been closed, stop printing in the future
        $DEBUG > 6 && print STDERR "    active=>$active< after line: $_";
        next;
    }

    next unless / uid="${interesting_uid}"/o;      # fast path skip only

    $DEBUG > 7 && print STDERR "  interesing uid $interesting_uid starts here: $_";
    
    if (/^\s*<(changeset|node|way|relation) .*?uid="${interesting_uid}"/o) {
        my $type=$1;
        print;  # always print start of changeset
        next if /\/>\s*$/o; # if changeset/node/way/relation XML tag is auto-closed in same line, no further action needed
        die "unparsable line $_" unless />\s*$/o;
        $active = $type;    # print everything until the end of the changeset (and remember what tag needs to be closed)
        $DEBUG > 5 && print STDERR "  found open XML element, printing until end of element: $active\n";
    } else {
        die "confusing line: $_";
    }
}