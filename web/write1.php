<?php
    // log in vao database
    include("config.php");

    // doc user input
    
    //$State_Windown = $_POST["State_Windown"];
    //$Mode_Windown = $_POST["Mode_Windown"];
    $Percent_Windown = $_POST["Percent_Windown"];

    // update lai database

    $sql = "update Window set Percent = $Percent_Windown";
    mysqli_query($conn, $sql);

    mysqli_close($conn);
?>