<?php
  // Settings for german version:
$status = new Status("LIVE Plugin State",
					 "Current stable version",
					 "0.2.0",
					 "m/d/Y",
					 "Language: ");

$menu = new Menu(array("index.php" => "About LIVE",
					   "news.php" => "News",
					   "download.php" => "Download",
					   "screenshots.php" => "Screenshots",
					   "index.php#links" => "Links and contact"
					 ));

$screenshots = new Screenshots(array("whatson" => "What's on (Extended view)",
									 "whatsonlist" => "What's on (List view)",
									 "schedule" => "Schedule of one channel for the next days",
									 "edittimer" => "Edit timer",
									 "search" => "Search through EPG data",
									 "searchtimer" => "Search timer",
									 "editsearchtimer" => "Edit search timer",
									 "timerconflict" => "Timer conflict display",
									 "recordings" => "Recordings",
									 "remote" => "Remote",
									 "setup" => "LIVE Settings",
								     "favourites" => "What's on (Favorites)"));
?>
