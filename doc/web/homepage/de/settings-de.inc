<?php
  // Settings for german version:
$status = new Status("LIVE Plugin Status",
					 "Aktuelle stabile Version",
					 "0.2.0",
					 "d.m.Y",
					 "Sprache: ");

$menu = new Menu(array("index.php" => "&Uuml;ber LIVE",
					   "news.php" => "Neuerungen",
					   "download.php" => "Download",
					   "screenshots.php" => "Screenshots",
					   "index.php#links" => "Links und Kontakt"
					 ));

$screenshots = new Screenshots(array("whatson" => "Was l&auml;uft (Ausf&uuml;hrliche Ansicht)",
									 "whatsonlist" => "Was l&auml;uft (Listenansicht)",
									 "schedule" => "Programm eines Kanals f&uuml;r die n&auml;chsten Tage",
									 "edittimer" => "Timer editieren",
									 "search" => "Suche in den EPG Daten",
									 "searchtimer" => "Suchtimer",
									 "editsearchtimer" => "Suchtimer editieren",
									 "timerconflict" => "Timer Konflikt Anzeige",
									 "recordings" => "Aufnahmen",
									 "remote" => "Fernbedienung",
									 "setup" => "LIVE Einstellungen",
								     "favourites" => "Was l&auml;uft (Favoritenansicht)"));
?>
