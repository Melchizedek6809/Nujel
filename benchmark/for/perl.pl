#!/usr/bin/env perl
use warnings;
use strict;

my $ret = 0;
for(my $i = 0; $i< 10000000; $i++){
  $ret = $ret + $i;
}
print "This results in $ret";
