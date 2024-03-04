<?php
  $i = 0;
  echo("<table>");
  while ($i != 10) {
    $j = 0;
    echo("<tr>");
    while ($j != 10) {
      $product = ($i + 1) * ($j + 1);
      $color = $i == $j ? "yellow" : "orange";
      echo("<td style=\"color: ".$color."; border: 1px solid black; padding: 5px;\">".$product."</td>");
      $j++;
    }
    echo("</tr>");
    $i++;
  }
  echo("</table>");

  echo("<table>");
  for ($i = 0; $i != 10; $i++) {
    echo("<tr>");
    for ($j = 0; $j != 10; $j++) {
      $sum = $i == 0 && $j == 0 ? "+" : ($i + 1) + ($j + 1);
      $color = $i == 0 || $j == 0 ? "lime" : "green";
      echo("<td style=\"color: ".$color."; border: 1px solid black; padding: 5px;\">".$sum."</td>");
    }
    echo("</tr>");
  }
  echo("</table>");
?>
