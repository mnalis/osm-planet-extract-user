use strict;
use warnings;
use MCE::Stream;
use feature 'say';

# Define your search pattern
my $pattern = qr/<changeset /o;

#     MCE::Stream->init(
#        chunk_size => 1, max_workers => 4,
#
#        user_begin => sub {
#           print "## ", MCE->wid, " started\n";
#        },
#
#        user_end => sub {
#           print "## ", MCE->wid, " completed\n";
#        }
#     );


# Perform multithreaded search with mce_grep_f
my @matches = mce_stream sub { /$pattern/o }, \*STDIN;

say "FIXME broken result: " . scalar @matches;

# Print the matches found
#foreach my $match (@matches) {
#    print $match;
#}
