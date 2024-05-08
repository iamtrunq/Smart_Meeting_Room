<?php
header('Content-Type: application/json');

// dang nhap vao database
include("config.php");

// Doc gia tri RGB tu database
$sql = "select * from Sen";
$result = mysqli_query($conn,$sql);

$data0 = array();
foreach ($result as $row){
    $data0[] = $row;
}

mysqli_close($conn);
echo json_encode($data0);

?>