<?php

function bench(){
	 $ret = 0;
	 for($i = 0; $i < 10000000; $i++){
		if((($i % 3) == 0) || (($i % 5) == 0)){
			$ret += $i;
		}
         }
	 return $ret;
}
echo "The sum is: " . bench();
