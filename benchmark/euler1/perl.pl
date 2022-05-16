#!/usr/bin/env perl
use warnings;
use strict;

my $ret = 0;
for(my $i = 0; $i< 10000000; $i++){
    if((($i % 3) == 0) || (($i % 5) == 0)){
        $ret = $ret + $i;
    }
}
print "The sum is: $ret";
