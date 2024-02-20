 use MCE::Loop;
 use warnings;
 use feature 'say';

 MCE::Loop->init(
    max_workers => 8, use_slurpio => 1
 );

 my $pattern  = qr/<changeset /o;

 my @result = mce_loop_f {
    my ($mce, $slurp_ref, $chunk_id) = @_;

    ## Quickly determine if a match is found.
    ## Process slurped chunk only if true.

    if ($$slurp_ref =~ /$pattern/o) {
       my @matches;

       ## The following is fast on Unix. Performance degrades
       ## drastically on Windows beyond 4 workers.

       open my $MEM_FH, '<', $slurp_ref;
       binmode $MEM_FH, ':raw';
       while (<$MEM_FH>) { push @matches, $_ if (/$pattern/o); }
       close   $MEM_FH;

       ## Therefore, use the following construct on Windows.

       while ( $$slurp_ref =~ /([^\n]+\n)/mg ) {
          my $line = $1; # save $1 to not lose the value
          push @matches, $line if ($line =~ /$pattern/o);
       }

       ## Gather matched lines.

       MCE->gather(@matches);
    }

 } \*STDIN;

# print join('', @result);
 say scalar @result;
