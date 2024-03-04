<html>
    <head>
        <title>Управление книгами</title>
    </head>
    <body>
        <h1>Список книг</h1><a href="new.html">Добавить книгу</a>
        <table border="1">
        <tr>
            <td>#</td>
            <td>Автор</td>
            <td>Название</td>
            <td>Редактить</td>
        </tr><?php
        $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
        $sql = "SELECT * FROM books";
        $result = $conn->query($sql);
        while ($row = $result->fetch()) {
            echo "<tr>";
            echo "<td>".$row['id']."</td>";
            echo "<td>".$row['name']."</td>";
            echo "<td>".$row['title']."</td>";
            echo "<td><a href='edit.php?id=".$row['id']."&name=".$row['name']."&title=".$row['title'].">Редактировать</a></td>";
            echo "<td><a href='delete.php?id=".$row['id'].">Удалить</a></td>";
            echo "</tr>";
        }
        ?>
        </table>
    </body>
</html>