<html>
    <head>
        <title>Управление книгами</title>
    </head>
    <body>
        <table border="1">
        <tr>
            <td>dsad</td>
        </tr><?php
        $filename = "notebook_01.txt";
        $file_array = file($filename);
        foreach($file_array as &$line) {
            rtrim($line, "| \n");
            $line = str_replace($line, "|", "</td><td>");
            echo $line;
            echo "<br>";
        }
        ?>
        </table>
    </body>
</html>

    