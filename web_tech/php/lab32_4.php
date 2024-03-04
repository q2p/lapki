<?php
  $list_sites = [
    "www.yandex.ru",
    "www.rambler.ru",
    "www.google.com",
    "www.yahoo.com",
    "www.altavista.com"
  ];
  if (isset($_POST["site"])) {
    header("Location: https://".$_POST["site"]);
  }
?>
<form method="post" action="lab32_4.php">
  <select name="site">
    <?php
      foreach ($list_sites as $i) {
        echo("<option value=\"".$i."\">".$i."</option>");
      }
    ?>
  </select>
  <button type="submit">Перейти</button>
</form>
