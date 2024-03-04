<?php
    $name = $_POST["name"];
    $title = $_POST["title"];

    $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
    $sql = "INSERT INTO books (name, title) VALUES ('$name', '$title')";
    $affectedRowsNumber = $conn->exec($sql);
    if($affectedRowsNumber > 0 ) {
        echo "Data successfully added: name=$name  age= $title";  
    }
    echo "<br/>";
    echo "<a href='index.php'>Назад</a>";
?>