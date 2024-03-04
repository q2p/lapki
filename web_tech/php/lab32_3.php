<?php
  $otv = [
    "russia" => "moscow",
    "usa" => "washington"
  ];
  $grade = 0;
  foreach ($otv as $country => $capital) {
    if ($_POST[$country] == $capital) {
      $grade++;
    }
  }
  $name = $_POST["name"];
  echo("Имя испытуемого: ".$name."<br>Баллов: ".$grade." из ".count($otv));
?>
