<html>
  <head>
    <title>Редактирование книги</title>
  </head>
  <body>
  <?php
    $conn = new PDO("mysql:host=localhost;dbname=library;charset=utf8mb4", "root", "");
    $rows = $conn->query("SELECT author, title FROM books WHERE id=".$_GET["id"]);
    while ($stroka=$rows->fetch()) {
      echo("<form action=lab41_save_edit.php method=post>");
      echo("<input type=hidden name=id value=\"".$_GET["id"]."\">");
      echo("Автор <input name=author value=\"".$stroka["author"]."\"><br>");
      echo("Название <input name=title value=\"".$stroka["title"]."\"><br>");
      echo("<input type=submit value=\"Сохранить\">");
      echo("</form>");
    }
  ?>
  </body>
</html>
