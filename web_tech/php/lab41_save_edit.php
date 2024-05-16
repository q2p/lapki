<body>
  <?php
    $conn = new PDO("mysql:host=localhost;dbname=library;charset=utf8mb4", "root", "");
    try {
      $rows = $conn->query("UPDATE books SET author='".$_POST["author"]."', title='".$_POST["title"]."' WHERE id=".$_POST["id"]);
      echo("Всё замечательно сохранено.<br>");
    } catch (PDOException $e) {
      echo("Ошибочка вышла.<br>");
    }
  ?>
  <a href="lab41_index.php">Вернуться к списку книг</a>
</body>
