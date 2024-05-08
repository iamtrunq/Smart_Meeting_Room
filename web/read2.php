<?php
header('Content-Type: application/json');

// dang nhap vao database
include("config.php");

// Doc gia tri RGB tu database
$sql = "select * from Fan";
$result = mysqli_query($conn,$sql);

$data2 = array();
foreach ($result as $row){
    $data2[] = $row;
}

mysqli_close($conn);
echo json_encode($data2);

?>