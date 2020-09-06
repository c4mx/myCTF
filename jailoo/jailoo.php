<?php 
	if(sizeof($_REQUEST)===2&& sizeof($_POST)===2){
	$cmd=$_POST['cmd'];
	$submit=$_POST['submit'];
	if(isset($cmd)&& isset($submit)){
	    if(preg_match_all('/^(\$|\(|\)|\_|\[|\]|\=|\;|\+|\"|\.)*$/', $cmd, $matches)){
	        echo "<div class=\"success\">Command executed !</div>";
            echo "<pre>" . $cmd . "</pre>";
	        eval($cmd);
	    }else{
	        die("<div class=\"error\">NOT ALLOWED 0 !</div>");
	    }
	}else{
	    die("<div class=\"error\">NOT ALLOWED 1 !</div>");
	}
	}else if ($_SERVER['REQUEST_METHOD']!="GET"){
	    die("<div class=\"error\">NOT ALLOWED 2 !</div>");
	}
?>
