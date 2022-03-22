<?php
/*
    <flag> ➡➡➡ ⛳🏁 ⬅⬅⬅ <flag>
*/
if ($_SERVER['REQUEST_METHOD'] == 'POST'){
    extract($_POST);
    var_dump($_POST['password']);
    var_dump(md5($_POST['password']));
    if (isset($_POST['password']) && md5($_POST['password']) == 'put hash here!'){
        echo "\nOK\n";
        $loggedin = true;
    }

    if (md5($_SERVER['REMOTE_ADDR']) != '92d3fd4057d07f38474331ab231e1f0d'){
        header('Location: ' . $_SERVER['REQUEST_URI']);
    }

    if (isset($loggedin) && $loggedin){
        echo 'One step closer 😎<br>';
        if (isset($_GET['action']) && md5($_GET['action']) == $_GET['action']){
            echo 'Really? 😅<br>';

            $db = new SQLite3('database.db');
            $sql_where = Array('1=0');

            foreach ($_POST as $key => $data) {
                $sql_where[] = $db->escapeString($key) . "='" . $db->escapeString($data) . "'";
            }
            var_dump('SELECT login FROM users WHERE ' . implode(' AND ', $sql_where));
            var_dump(__FILE__);
            $result = $db->querySingle('SELECT login FROM users WHERE ' . implode(' AND ', $sql_where));

            if ($result == 'admin'){
                echo 'Last step 🤣<br>';
                readfile(file_get_contents('php://input'));
            }
        }
    }
}
?>

<?php
    var_dump(__FILE__);
    $source = highlight_file(__FILE__, true);
    $source = explode('&lt;flag&gt;', $source);
    $source[1] = ' ➡➡➡ ⛳🏁 ⬅⬅⬅ ';
    $source = implode('&lt;flag&gt;', $source);
    echo $source;
?>