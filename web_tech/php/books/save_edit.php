<?php

    $id = $_POST['id'];
    $name = $_POST['name'];
    $title = $_POST['title'];
    
    $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
    $sql = "UPDATE books SET name = :name, title = :title WHERE id = :id";
    $stmt = $conn->prepare($sql);
    $stmt->bindValue(":name", $name);
    $stmt->bindValue(":title", $title);
    $stmt->bindValue(":id", $id);
    $stmt->execute();
    header("Location: index.php");