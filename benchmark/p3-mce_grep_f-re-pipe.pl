use strict;
use warnings;
use MCE::Grep;
use feature 'say';

# Define your search pattern
my $pattern = qr/<changeset /o;

# Perform multithreaded search with mce_grep_f
my $matches = mce_grep_f { '.' if /$pattern/o } \*STDIN;

say $matches;

#say scalar @matches;

# Print the matches found
#foreach my $match (@matches) {
#    print $match;
#}
