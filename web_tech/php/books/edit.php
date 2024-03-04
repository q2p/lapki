<html>
    <head>
        <title>Управление книгами</title>
    </head>
    <body>
        <?php
            $id = $_GET['id'];
            $name = $_GET['name'];
            $title = $_GET['title'];
        ?>

        <form action="save_edit.php" method="post">
            <input type="text" name="name" placeholder="Новый автор" value="<?php echo $name; ?>">
            <label for="name">Name</label>
            <br />
            <input type="text" name="title" placeholder="Новое название книги" value="<?php echo $title; ?>">
            <label for="title">Title</label>
            <br />
            <input type="text" name="id" value="<?php echo $id; ?>">
            <button type="submit">Save!</button>
        </form>

        <a href="save_delete.php?id=<?php echo $id; ?>">Delete!</a>
    </body>
</html>
