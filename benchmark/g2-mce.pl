use strict;
use warnings;
use feature 'say';
use MCE::Grep;

open(my $LOG, '<', '/tmp/h.osm');
my $count = mce_grep { /<changeset /o } $LOG;
say scalar $count;
