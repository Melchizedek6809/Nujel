<?php

function reverseNum($a){
	$ret = 0;
	while($a > 0){
		$ret = ($ret * 10) + ($a % 10);
		$a = intdiv($a, 10);
	}
	return $ret;
}

function palindromeP($a){
	return $a == reverseNum($a);
}

function startSearch(){
	$ret = 0;
	for($a=0;$a<1000;$a++){
		for($b=0;$b<1000;$b++){
			$p = $a * $b;
			if(palindromeP($p) && ($p > $ret)){
				$ret = $p;
			}
		}
	}
	return $ret;
}

echo("The biggest product of 2 3-digit numbers that is a palindrome is: " . startSearch());
