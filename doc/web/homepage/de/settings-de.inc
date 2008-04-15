    <?php
       // Settings for german version:
class Status {
	var $title = "LIVE Plugin Status";
	var $vers_string = "Aktuelle stabile Version";
	var $vers_number = "0.2.0";
        var $datefmt = "d.m.Y";
}

$status = new Status;

class Menu {
	var $urls = array(
		"index.php" => "&Uuml;ber Live",
		"news.php" => "Neuerungen",
		"download.php" => "Download",
		"screenshots.php" => "Screenshots",
		"index.php#links" => "Links und Kontakt"
		);
}

$menu = new Menu;

       ?>
