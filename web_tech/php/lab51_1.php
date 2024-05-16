<?php
  $filename = "notebook_01.txt";

  $conn = new PDO("mysql:host=localhost;dbname=webtechdb", "root", "");
  $result = $conn->query("SELECT * FROM notebook_01");

  $fd = fopen($filename, 'w');
  while ($row = $result->fetch()) {
    fputs($fd, $row['id']);
    fputs($fd, " | ");
    fputs($fd, $row['name']);
    fputs($fd, " | ");
    fputs($fd, $row['city']);
    fputs($fd, " | ");
    fputs($fd, $row['address']);
    fputs($fd, " | ");
    $date = preg_replace("/(\d+)-(\d+)-(\d+)/i", "\$3.\$2.\$1", $row['birthday']);
    fputs($fd, $date);
    fputs($fd, " | ");
    fputs($fd, $row['mail']);
    fputs($fd, "\n");
  }
  fflush($fd);
  fclose($fd);

  $fd = fopen($filename, "r");
  while (!feof($fd)) {
    $line = fgets($fd);
    echo($line."<br>");
  }
?>
