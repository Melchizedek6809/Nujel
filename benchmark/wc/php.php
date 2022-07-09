<?php

$content = file_get_contents("benchmark/bib.txt");
$nl = 0;
$nw = 0;
$nc = strlen($content);
$inWord = false;

for($i=0; $i<$nc; $i++){
    switch($content[$i]){
        case "\n":
            $nl++;
            $inWord = false;
            break;
        case " ":
            $inWord = false;
            break;
        default:
            if(!$inWord){
                $nw++;
            }
            $inWord = true;
            break;
    }
}

echo("Lines: ".$nl."\n");
echo("Words: ".$nw."\n");
echo("Characters: ".$nc."\n");