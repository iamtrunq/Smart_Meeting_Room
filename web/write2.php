<?php
    // log in vao database
    include("config.php");

    // doc user input
    
    $State_Fan = $_POST["State_Fan"];

    $sql = "update Fan set State = $State_Fan";
    mysqli_query($conn, $sql);

    mysqli_close($conn);
?>