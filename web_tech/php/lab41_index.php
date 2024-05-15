<html>
  <head>
    <title>Управление книгами</title>
  </head>
  <body>
  <h1>Список книг</h1>
  <a href="lab41_new.html">Добавить книгу</a>
  <table border="1">
    <tr>
      <td>#</td>
      <td>Автор</td>
      <td>Название</td>
      <td>Редактировать</td>
      <td>Уничтожить</td>
    </tr>
      <?php
        $conn = new PDO("mysql:host=localhost;dbname=library;charset=utf8mb4", "root", "");
        $rows = $conn->query("SELECT id, author, title FROM books");
        while ($stroka = $rows->fetch()) {
          echo("<tr>");
          echo("<td>" . $stroka["id"] . "</td>");
          echo("<td>" . $stroka["author"] . "</td>");
          echo("<td>" . $stroka["title"] . "</td>");
          echo("<td><a href=\"lab41_edit.php?id=" . $stroka["id"] . "\">Редактировать</a></td>");
          echo("<td><a href=\"lab41_delete.php?id=" . $stroka["id"] . "\">Удалить</a></td>");
          echo("</tr>");
        }
      ?>
    </table>
  </body>
</html>
