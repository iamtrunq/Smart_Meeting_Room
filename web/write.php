<?php
    // log in vao database
    include("config.php");

    // doc user input
    
    $State_Lamp = $_POST["State_Lamp"];

    // update lai database
    $sql = "update Light set State = $State_Lamp";
    mysqli_query($conn, $sql);

    mysqli_close($conn);
?>