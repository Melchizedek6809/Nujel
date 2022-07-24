#!/usr/bin/env perl
use warnings;
use strict;

sub reverseNum {
    my $a = $_[0];
    my $ret = 0;
    while ($a > 0){
        $ret = ($ret * 10) + ($a % 10);
        $a = int $a / 10;
    }
    return $ret;
}

sub startSearch {
    my $ret = 0;
    for(my $a=0; $a < 1000; $a++){
        for(my $b=0; $b < 1000; $b++){
            my $p = $a * $b;
            if((reverseNum($p) == $p) && ($p > $ret)){
                $ret = $p;
            }
        }
    }
    print "The biggest product of 2 3-digit numbers that is a palindrome is: $ret";
}

startSearch();
