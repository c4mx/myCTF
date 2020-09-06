<?php

$cmd=$argv[1];

if(preg_match_all('/^(\$|\(|\)|\_|\[|\]|\=|\;|\+|\"|\.)*$/', $cmd, $matches)) {
    echo $matches[0][0] . ", " . $matches[0][1] . "\n";
    echo $matches[1][0] . ", " . $matches[1][1] . "\n";
    echo "<div class=\"success\">Command executed !</div>\n";
    echo "cmd: " . $cmd . "\n";
    eval($cmd);

}else{

    echo $matches[0][0] . ", " . $matches[0][1] . "\n";
    echo $matches[1][0] . ", " . $matches[1][1] . "\n";
    die("<div class=\"error\">NOT ALLOWED !</div>");

}

?>
