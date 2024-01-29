use strict;
use warnings;
use feature 'say';

open(my $LOG, '<', '/tmp/h.osm');
my $count = grep { index($_, '<changeset ',) >= 0 } <$LOG>;
say scalar $count;
