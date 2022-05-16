<?php

$ret = 0;
for($i = 0; $i < 10000000; $i++){
	if((($i % 3) == 0) || (($i % 5) == 0)){
		$ret += $i;
	}
}
echo "The sum is: " . $ret;
