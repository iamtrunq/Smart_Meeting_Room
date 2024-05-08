<?php
    // log in vao database
    include("config.php");

    // doc user input
    
    
    $Set_Lamp = $_POST["Set_Lamp"];
    

    // update lai database
    $sql = "update Light set Set_Lux = $Set_Lamp";
    mysqli_query($conn, $sql);

    mysqli_close($conn);
?>