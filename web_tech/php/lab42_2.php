<?php
    $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");

    $name;
    $city;
    $address;
    $date;
    $mail;
    if (isset($_POST["name"])) {
        $name = $_POST["name"];
    }
    if (isset($_POST["city"])) {
        $city = $_POST["city"];
    }
    if (isset($_POST["address"])) {
        $address = $_POST["address"];
    }
    if (isset($_POST["date"])) {
        $date = $_POST["date"];
    }
    if (isset($_POST["mail"])) {
        $mail = $_POST["mail"];
    }
    
    $sql = "INSERT INTO notebook_01 (name, city, address, birthday, mail)
    VALUES ('$name', '$city', '$address', '$date', '$mail')";
    $conn->exec($sql);