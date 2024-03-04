<?php
  function section($num) {
    echo("<h3>".$num."</h3>");
  }

  section("1, 2");
  $color = "#0011ff";
  $size = 24;
  echo("<div style=\"color: ".$color."; font-size: ".$size."px\">Цветной текст</div>");

  section("3");
  $str = "student";
  $$str = "user";

  print "$str $user";
  echo("<br>");
  print "str"; print "$$str";
  echo("<br>");
  print $str; print $$str;
  echo("<br>");

  section("4");
  $str1 = "student";
  $str2 = &$str1;
  echo($str2."<br>");
  $str1 = "user";
  echo($str2."<br>");

  section("5, 6");
  const NUM_E = 2.71828;
  echo("Число E равно ".NUM_E);

  section("7");
  $num_e1 = NUM_E;
  echo("Тип константы: ".gettype($num_e1)."<br>");

  section("8");
  $type_and_val = function($num) {
    echo("Тип: ".gettype($num).", значение: ".$num.".<br>");
  };
  $num_e1 = (string) $num_e1;
  $type_and_val($num_e1);
  $num_e1 = (int) $num_e1;
  $type_and_val($num_e1);
  $num_e1 = (bool) $num_e1;
  $type_and_val($num_e1);
?>
