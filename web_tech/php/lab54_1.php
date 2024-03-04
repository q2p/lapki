<p>Ваша корзина</p>
<?php
  session_start();
  echo("<ul>");
  foreach($_SESSION['items'] as $item) {
    echo("<li>".$item."</li>");
  }
  echo("</ul>");
?>
