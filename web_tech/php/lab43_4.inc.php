<?php  
    $table = "";
    if ($_POST['structure'] == $_POST['content']) {
        $table = $_POST['structure'];
    }
    function view_structure($table) {
        echo "<h4>Структура таблицы '$table'</h4>";
        echo "<table border='1'>";
        echo "<tr>";
        echo "<td>Id</td>";
        echo "<td>Name</td>";
        echo "</tr>";
        echo "</table>";
    }
    function view_content($table) {
        $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
        $sql = "SELECT * FROM $table";
        $result = $conn->query($sql);
        echo "<h4>Контент таблицы '$table'</h4>";
        echo "<table border='1'>";
        while ($row = $result->fetch()) {
            $id = $row['id'];
            $name = $row['name'];
            echo "<tr>";
            echo "<td>'$id'</td>";
            echo "<td>'$name'</td>";
            echo "</tr>";
        }
        echo "</table>";
    }
    view_structure($table);
    view_content($table);
    echo "<a href='lab43_1.html'>Назад</a>";