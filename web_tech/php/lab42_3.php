<html>
    <head>
        <title>Управление книгами</title>
    </head>
    <body>
        <table border="1">
        <tr>
            <td>id</td>
            <td>name</td>
            <td>city</td>
            <td>address</td>
            <td>birthday</td>
            <td>mail</td>
        </tr><?php
        $conn = new PDO("mysql:host=localhost;dbname=library41", "root", "");
        $sql = "SELECT * FROM notebook_01";
        $result = $conn->query($sql);
        while ($row = $result->fetch()) {
            echo "<tr>";
            echo "<td>".$row['id']."</td>";
            echo "<td>".$row['name']."</td>";
            echo "<td>".$row['city']."</td>";
            echo "<td>".$row['address']."</td>";
            echo "<td>".$row['birthday']."</td>";
            echo "<td>".$row['mail']."</td>";
            echo "</tr>";
        }
        ?>
        </table>
    </body>
</html>