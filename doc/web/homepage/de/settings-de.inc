<?php
  // Settings for german version:
$status = new Status("LIVE Plugin Status",
					 "Aktuelle stabile Version",
					 "0.2.0",
					 "d.m.Y");

$menu = new Menu(array("index.php" => "&Uuml;ber Live",
					   "news.php" => "Neuerungen",
					   "download.php" => "Download",
					   "screenshots.php" => "Screenshots",
					   "index.php#links" => "Links und Kontakt"
					 ));

$screenshots = new Screenshots(array("whatson",
									 "schedule",
									 "timers",
									 "searchepg",
									 "edit_searchtimer",
									 "recordings",
									 "remote",
									 "setup"));
?>
