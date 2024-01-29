use strict;
use warnings;
use feature 'say';
use MCE::Grep;

my $count = mce_grep_f { /<changeset /o } '/tmp/h.osm';
say scalar $count;
