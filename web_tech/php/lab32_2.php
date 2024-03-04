<?php
  function cif($selector, $color) {
    if ($_POST[$selector] == $color) {
      echo("checked");
    }
  }
?>
<form method="post" action="lab32_2.php">
  <fieldset>
    <legend>Цвет текста</legend>
    <input type="radio" id="fg_red" name="color" value="red" <?php cif("color", "red") ?>><label for="fg_red">Красный</label><br>
    <input type="radio" id="fg_blue" name="color" value="blue" <?php cif("color", "blue") ?>><label for="fg_blue">Синий</label><br>
    <input type="radio" id="fg_green" name="color" value="green" <?php cif("color", "green") ?>><label for="fg_green">Зелёный</label><br>
  </fieldset>

  <fieldset>
    <legend>Цвет фона</legend>
    <input type="radio" id="bg_red" name="bgcolor" value="red" <?php cif("bgcolor", "red") ?>><label for="bg_red">Красный</label><br>
    <input type="radio" id="bg_blue" name="bgcolor" value="blue" <?php cif("bgcolor", "blue") ?>><label for="bg_blue">Синий</label><br>
    <input type="radio" id="bg_green" name="bgcolor" value="green" <?php cif("bgcolor", "green") ?>><label for="bg_green">Зелёный</label><br>
  </fieldset>

  <fieldset>
    <legend>Горизонтальное выравнивание</legend>
    <input type="checkbox" id="left" name="align" value="left" <?php cif("align", "left") ?>><label for="left">Лево</label><br>
    <input type="checkbox" id="center" name="align" value="center" <?php cif("align", "center") ?>><label for="center">Центр</label><br>
    <input type="checkbox" id="right" name="align" value="right" <?php cif("align", "right") ?>><label for="right">Право</label><br>
  </fieldset>
  <fieldset>
    <legend>Вертикальное выравниванние</legend>
    <input type="checkbox" id="top" name="valign" value="top" <?php cif("valign", "top") ?>><label for="top">Верх</label><br>
    <input type="checkbox" id="middle" name="valign" value="middle" <?php cif("valign", "middle") ?>><label for="middle">Середина</label><br>
    <input type="checkbox" id="bottom" name="valign" value="bottom" <?php cif("valign", "bottom") ?>><label for="bottom">Низ</label><br>
  </fieldset>
  <button type="submit">Выполнить</button>
</form>
<?php
  include("./lab32_1.php");
?>
