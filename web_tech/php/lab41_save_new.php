<html>
  <head>
    <title></title>
  </head>
  <body>
    <?php
      $conn = new PDO("mysql:host=localhost;dbname=library;charset=utf8mb4", "root", "");
      try {
        $affected_rows = $conn->query("INSERT INTO books SET author=\"".$_POST["author"]."\", title=\"".$_POST["title"]."\"");
        echo("Всё замечательно сохранено.<br>");
      } catch (PDOException $e) {
        echo("Ошибочка вышла.<br>");
      }
    ?>
    <a href="lab41_index.php">Вернуться к спику книг</a>
  </body>
</html>
