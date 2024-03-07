<?php
  $conn = new PDO("mysql:host=localhost;dbname=library;charset=utf8mb4", "root", "");
  $rows = $conn->query("DELETE FROM books WHERE id=".$_GET["id"]);
  header("Location: lab41_index.php");
?>
