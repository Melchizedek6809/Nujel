<?php

$ret = 0;
for($i = 0; $i < 10000000; $i++){
	$ret += $i;
}
echo "The result is: " . $ret;
