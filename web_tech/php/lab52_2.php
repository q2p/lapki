<?php
  session_start();
  $_SESSION["name"] = $_POST["name"];
  echo "Вы успешно авторизовались";
?>
