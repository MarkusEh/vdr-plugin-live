<?php
require_once 'HTTP.php';
$langs = array("de" => true, "en" => true);
$selected = HTTP::negotiateLanguage($langs, "en");
header("Location: $selected/index.php");
exit;
?>
