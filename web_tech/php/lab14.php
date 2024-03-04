<?php
  function PrintLang($text, $color) {
    echo "<div style=\"color: ".$color.";\">".$text."!</div>";
  }
  function Ru($color) {
    PrintLang("Здравствуйте", $color);
  }
  function En($color) {
    PrintLang("Hello", $color);
  }
  function Fr($color) {
    PrintLang("Bonjour", $color);
  }
  function De($color) {
    PrintLang("GutenTag", $color);
  }

  $color = $_GET["color"];
  switch ($_GET["lang"]) {
    case "ru": Ru($color); break;
    case "en": En($color); break;
    case "fr": Fr($color); break;
    case "de": De($color); break;
    default: echo "язык неизвестен";
  }

  function WeekDay($day, $color) {
    global $size;
    $days = ["Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"];
    echo("<div style=\"color: ".$color."; font-size: ".$size."px;\">".$days[$day]."</div>");
  }

  $size = 7;
  for ($i = 0; $i != 7; $i++) {
    WeekDay($i, $color);
    $size--;
  }
?>
