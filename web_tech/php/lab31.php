<?php
  function section($header) {
    echo("<h3>".$header."</h3>");
  }
  function echo_arr($array) {
    for ($i = 0; $i != count($array); $i++) {
      echo($array[$i]." ");
    }
    echo("<br>");
  }

  section("3.1.1");
  $treug = [];
  for ($n = 1; $n <= 10; $n++) {
    $treug[$n-1] = $n*($n+1)/2;
  }
  echo_arr($treug);

  section("3.1.2");
  $kvd = [];
  for ($i = 0; $i != 10; $i++) {
    $kvd[$i] = ($i+1)**2;
  }
  echo_arr($kvd);

  section("3.1.3");
  $rez = array_merge($treug, $kvd);
  echo_arr($rez);

  section("3.1.4");
  sort($rez);
  echo_arr($rez);

  section("3.1.5");
  array_shift($rez);
  echo_arr($rez);

  section("3.1.6");
  $rez1 = array_unique($rez);
  echo_arr($rez1);

  section("3.1.7");
  echo "<table border=1 cellspacing=0 cellpadding=0><tbody>";
  for($i=0; $i != 30; $i++) {
    echo "<tr>";
    for($j=0; $j != 30; $j++) {
      $color = "";
      switch (($i+1) * ($j+1) % 7) {
        case 0: $color = "white";  break;
        case 1: $color = "aqua";   break;
        case 2: $color = "blue";   break;
        case 3: $color = "yellow"; break;
        case 4: $color = "purple"; break;
        case 5: $color = "red";    break;
        case 6: $color = "lime";   break;
      }
      echo "<td size=1 width=14 height=15 bgcolor=".$color.">&nbsp;</td>";
    }
    echo "</tr>";
  }
  echo "</tbody></table>";
?>
