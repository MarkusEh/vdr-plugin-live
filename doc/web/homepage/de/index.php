<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR LIVE - Home</title>
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
	<a name="about"></a>
	<div class="boxheader"><div><div>&Uuml;ber LIVE</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <div class="screenshots" style="float: right; border: 0; padding-top: 15px"><?php $screenshots->RandomImg(); ?></div>
	  <p>
	    <b>LIVE</b>
	  </p>
          <p>
            LIVE, das <strong><em>LIVE Interactive VDR
            Environment</em></strong>, erm&ouml;glicht die komfortable
            Bedienung des VDR und mancher seiner Plugins &uuml;ber ein
            Webinterface.</p>

          <p>Anders als externe Programme, die mit VDR und seinen
            Plugins &uuml;ber die SVDRP-Schnittstelle kommunizieren,
            hat LIVE einen direkten Zugriff auf die internen
            Datenstrukturen und ist dadurch sehr schnell.</p>

          <p>Zudem wurde eine Schnittstelle eingef&uuml;hrt, die eine
            direkte Integration einer Bedienoberfl&auml;che f&uuml;r
            VDR Plugins in LIVE erm&ouml;glicht.<br/>
            Gegenw&auml;rtig wurde dies
            f&uuml;r <a href="http://winni.vdr-developer.org/epgsearch/index.html">EPGSearch</a>
            realisiert, welches einen erheblichen Teil des
            Funktionsumfangs der Weboberfl&auml;che zur Verf&uuml;gung
            stellt. Langfristig ist das f&uuml;r andere Plugins, wie
            zum Beispiel taste, burn, femon und viele andere
            geplant.<br/>  LIVE funktioniert aber auch ohne diese(s)
            Plugin(s). LIVE erkennt das Fehlen eines Plugins und passt
            seine Oberfl&auml;che entsprechend an. Daher kann es sein,
            dass bestimmte Funktionen erst bei Aktivierung der Plugins
            sicht- und verwendbar werden.
          </p>
          <p>
	    Einen ersten optischen Eindruck von LIVE kann man sich auf
	    der <a href="screenshots.php"><?php echo
	    $menu->urls["screenshots.php"]; ?>-Seite</a> verschaffen.
	  </p>
	  <p>
	    <b>Lizenz</b>
	  </p>
	  <p>
	    LIVE wird unter
	    der <a href="http://www.gnu.org/licenses/old-licenses/gpl-2.0.html">GNU
	    General Public License, Version 2</a> (GPLv2)
	    ver&ouml;ffentlicht.  N&auml;here Informationen erhalten
	    Sie durch verfolgen des Links und/oder in der
	    Datei <tt>COPYING</tt> im Quellcode Verzeichnis von LIVE.
	  </p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Neue Features von LIVE <?php echo $status->vers_number; ?></div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <?php include ("features_0-2-0.inc"); ?>
	  <p>
	    Alle bisherigen &Auml;nderungen an LIVE sind auf der Seite
	    mit den <a href="news.php">archivierten <?php echo
	    $menu->urls["news.php"];?></a> aufgelistet.
	  </p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Direkter Download von LIVE <?php echo $status->vers_number; ?></div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Version <?php echo $status->vers_number; ?></b>
            <ul>
	      <li>
		Download: <?php echo "<a href=\"http://live.vdr-developer.org/downloads/vdr-live-" . $status->vers_number . ".tar.gz\">" . "http://live.vdr-developer.org/downloads/vdr-live-" . $status->vers_number . ".tar.gz</a>"; ?>
	        <p>
		  <strong>WICHTIG:</strong> unbedingt auch immer die Datei README in LIVE Quellen-Verzeichnis beachten!
		</p>
	      </li>
            </ul>
	  </p>
	  <p>
	    <b>Vorausetzungen f&uuml;r die Installation:</b>
	    <?php include ("install_0-2-0.inc"); ?>
	  </p>
          <p>
	    Andere Versionen von LIVE k&ouml;nnen von
	    der <a href="download.php"><?php echo
	    $menu->urls["download.php"]; ?>-Seite</a> herunter geladen
	    werden.
	  </p>
        </div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<a name="links"></a>
	<div class="boxheader"><div><div>Links</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
          <p>
            <b>Web:</b>
            <ul>
              <li><a href="http://www.vdr-developer.org/mantisbt/main_page.php">Bugtracker</a> - Bugs und Featurew&uuml;nsche</li>
              <li><a href="http://www.cadsoft.de/vdr/">Video Disk Recorder - VDR</a></li>
              <li><a href="http://winni.vdr-developer.org/epgsearch/index.html">EPGSearch Plugin</a></li>
              <br />
              <li><a href="http://www.vdr-portal.de">VDR-Portal</a></li>
              <li><a href="http://www.vdr-wiki.de">VDR-Wiki</a></li>
            </ul>
          </p>
          <p>
            <b>IRC:</b>
            <ul>
              <li>Server: irc://www.vdr-portal.de</li>
              <li>Channel: #live</li>
            </ul>
          </p>
          <p>
            <b>Sonstiges</b>
            <ul>
              <li>Zus&auml;tzliche Kontaktinfos finden Sie am Anfang der README Datei im LIVE Quellcode.</li>
            </ul>
          </p>
	</div>
      </div>

    </div>
  </body>
</html>
