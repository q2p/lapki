<body>
  <?php
    $conn = new PDO("mysql:host=localhost;dbname=sample;charset=utf8mb4", "root", "");

    $table_name = "notebook_".$_POST["brigade"];

    $conn->exec("DROP TABLE IF EXISTS ".$table_name);
    try {
      $conn->exec("CREATE TABLE ".$table_name."(
        id integer AUTO_INCREMENT PRIMARY KEY,
        name VARCHAR(50),
        city VARCHAR(50),
        address VARCHAR(50),
        birthday DATE,
        mail VARCHAR(20)
      );");
      echo("Всё замечательно сохранено.<br>");
    } catch (PDOException $e) {
      echo("Нельзя создать таблицу ".$table_name);
    }
  ?>
  <a href="lab42_index.php">Вернуться к списку книг</a>
</body>
