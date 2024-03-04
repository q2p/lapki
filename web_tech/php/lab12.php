<?php
  $lang = $_GET["lang"];

  if ($lang == "ru") {
    echo("русский");
  } else if ($lang == "en") {
    echo("английский");
  } else if ($lang == "fr") {
    echo("французский");
  } else if ($lang == "de") {
    echo("немецкий");
  } else {
    echo("язык неизвестен");
  }
  echo("<br>");

  switch ($lang) {
    case "ru":
      echo("русский");
      break;
    case "en":
      echo("английский");
      break;
    case "fr":
      echo("французский");
      break;
    case "de":
      echo("немецкий");
      break;
    default:
      echo("язык неизвестен");
  }
  echo("<br>");

  echo($lang == "ru" ? "Привет" : ($lang == "en" ? "Hello" : ($lang == "de" ? "Hallo" : ($lang == "fr" ? "Bonjour" : "язык неизвестен"))));
?>
