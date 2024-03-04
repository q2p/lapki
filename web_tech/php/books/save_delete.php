<?php

    $id = $_GET['id'];

    $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
    $sql = "DELETE from books WHERE id = :id";
    $stmt = $conn->prepare($sql);
    $stmt->bindValue(":id", $id);
    $stmt->execute();
    header("Location: index.php");