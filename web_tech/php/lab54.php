<?php
  session_start();
  $item = $_POST["items"];
  if(!isset($_SESSION["items"])) {
    $_SESSION["items"] = [];
  }
  if (!in_array($item, $_SESSION["items"])) {
    array_push($_SESSION["items"], $item);
  }
?>
