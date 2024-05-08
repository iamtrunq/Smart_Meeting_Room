<?php
    // log in vao database
    include("config.php");

    // doc user input
    
    $Mode = $_POST["Mode"];
    

    // update lai database
    $sql = "update Light set Mode = $Mode";
    mysqli_query($conn, $sql);
    $sql1 = "update Window set Mode = $Mode";
    mysqli_query($conn, $sql1);

    mysqli_close($conn);
?>