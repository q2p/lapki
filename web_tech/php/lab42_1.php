<?php
    $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
    $sql = "DROP TABLE IF EXISTS notebook_01";
    $conn->exec($sql);

    try {
        $sql = "CREATE TABLE notebook_01 (id integer AUTO_INCREMENT PRIMARY KEY, 
        name VARCHAR(50), city VARCHAR(50), address VARCHAR(50), birthday DATE, mail VARCHAR(20));";
        $conn->exec($sql);
        echo "Db created";
    } catch (PDOExeption $e) {
        echo "Database error: " . $e->getMessage();
    }