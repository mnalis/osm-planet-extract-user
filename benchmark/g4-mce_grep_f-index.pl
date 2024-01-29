use strict;
use warnings;
use feature 'say';
use MCE::Grep;

my $count = mce_grep_f { index($_, '<changeset ') >= 0 } '/tmp/h.osm';
say scalar $count;
