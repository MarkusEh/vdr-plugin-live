<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR LIVE - Downloads</title>
    <?php include ("../html-header.inc") ?>
  </head>
  <body>
    <?php
       include ("../page-php-classes.inc");
       include ("settings-de.inc");
    ?>
    <div class="page_header">
      <?php include ("../page-header.inc"); ?>
    </div>
    <div style="clear:both"></div>

    <?php include ("../page-menu.inc"); ?>

    <div class="inhalt">

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Stabile Version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Version 0.2.0</b>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <b>Vorausetzungen f&uuml;r die Installation:</b>
	    <?php include ("install_0-2-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Entwickler Version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Akuteller CVS-Snapshot</b>
            <ul>
	      <li>
		Download: <a href="http://www.vdr-developer.org/cgi-bin/cvsweb.cgi/live/live.tar.gz?tarball=1">http://www.vdr-developer.org/cgi-bin/cvsweb.cgi/live/</a>
	        <p>
		  <strong>WICHTIG:</strong> unbedingt auch immer die Datei README in LIVE Quellen-Verzeichnis beachten!
		</p>
	      </li>
            </ul>
	  </p>
	  <p>
	    <b>Vorausetzungen f&uuml;r die Installation:</b>
	    <?php include ("install_devel.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>&Auml;ltere Version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Version 0.1.0</b>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <b>Vorausetzungen f&uuml;r die Installation:</b>
	    <?php include ("install_0-1-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

    </div>
  </body>
</html>
