use strict;
use warnings;
use MCE::Grep;
use feature 'say';

# Perform multithreaded search with mce_grep_f
my $matches = mce_grep_f { index($_, '<changeset ') >= 0 } \*STDIN;

say $matches;

#say scalar @matches;

# Print the matches found
#foreach my $match (@matches) {
#    print $match;
#}
