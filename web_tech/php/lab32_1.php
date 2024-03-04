<table>
  <tbody>
    <tr>
    <?php
      $fg_color = $_POST["color"];
      $bg_color = $_POST["bgcolor"];
      $align = $_POST["align"];
      $valign = $_POST["valign"];
      echo("<td bgcolor=".$bg_color." style=\"color: ".$fg_color."\" width=100 height=100 align=".$align." valign=".$valign.">Текст</td>");
    ?>
    </tr>
  </tbody>
</table>
<a href="/lab32.html">Назад</a>
