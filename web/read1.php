<?php
header('Content-Type: application/json');

// dang nhap vao database
include("config.php");

// Doc gia tri RGB tu database
$sql = "select * from Window";
$result = mysqli_query($conn,$sql);

$data1 = array();
foreach ($result as $row){
    $data1[] = $row;
}

mysqli_close($conn);
echo json_encode($data1);

?>