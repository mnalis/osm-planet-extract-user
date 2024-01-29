use strict;
use warnings;
use feature 'say';

open(my $LOG, '<', '/tmp/h.osm');
my $count = grep { /<changeset /o } <$LOG>;
say scalar $count;
