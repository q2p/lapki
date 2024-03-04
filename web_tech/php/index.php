<?php
  $color = "#0011ff";
  $size = 24;
  echo("<p style='color:".$color."; font-size: ".$size."px'>Hello world</p>");
?>

    // 1.1 4
    // $str1 = "student";
    // $str2 = &$str1;
    // echo($str2." ");
    // $str1 = "user";
    // echo($str2);

    // 1.1 5 and 6
    const NUM_E = 2.71828;
    // echo "Число e равно ". NUM_E;

    // 1.1 7
    $num_e1 = NUM_E;
    // echo gettype($num_e1);

    // 1.1 8
    // var_dump($num_e1);
    // echo "<br/>";
    // var_dump((string)$num_e1);
    // echo "<br/>";
    // var_dump((int)$num_e1);
    // echo "<br/>";
    // var_dump((bool)$num_e1);

    $str = "student";
    $$str = "user";
    // print "$str $user";
    // print "str"; print "$$str";
    print $str;print $$str;
