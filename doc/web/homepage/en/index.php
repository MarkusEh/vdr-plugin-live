<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR LIVE - Home</title>
    <?php include ("../html-header.inc") ?>
  </head>
  <body>
    <?php
       include ("../page-php-classes.inc");
       include ("settings-en.inc");
    ?>
    <div class="page_header">
      <?php include ("../page-header.inc"); ?>
    </div>
    <div style="clear:both"></div>

    <?php include ("../page-menu.inc"); ?>

    <div class="inhalt">

      <div style="width: 800px; margin-top: 15px">
	<a name="about"></a>
	<div class="boxheader"><div><div>About LIVE</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <div class="screenshots" style="float: right; border: 0; padding-top: 15px"><?php $screenshots->RandomImg(); ?></div>
	  <p>
	    <strong>LIVE</strong>
	  </p>
          <p>
            LIVE, the <strong><em>LIVE Interactive VDR
            Environment</em></strong>, allows a comfortable operation
            of VDR and some of its plugins trough a web interface.
	  </p>
          <p>
	    Unlike external programs, which communicate with VDR and
	    its plugins through the SVDRP interface, LIVE has direct
	    access to the internal data structures of VDR. This makes
	    LIVE very fast.
	  </p>
          <p>
	    Additionally LIVE introduces an interface which allows the
	    direct integration of user interfaces for VDR
	    plugins.<br/>  Currently this interface was implemented
	    for
	    the <a href="http://winni.vdr-developer.org/epgsearch/index.html">EPGSearch</a>
	    plugin. A substantial part of the LIVE web interface is
	    based on this implementation. There exist plans to extend
	    other plugins (like taste, burn, femon and others) with
	    this interface too.<br/>  LIVE works even when these
	    plugins are missing. LIVE detects the available plugins
	    and adapts its user interface accordingly. Therefor it can
	    happen, that certain functions appear and can be used only
	    when the appropriate plugin is activated in VDR.
          </p>
          <p>
	    You can get a first optical impression of LIVE on the <a href="screenshots.php"><?php echo
	    $menu->urls["screenshots.php"]; ?> page</a>
	  </p>
	  <p>
	    <strong>License</strong>
	  </p>
	  <p>
	    LIVE is distributed under
	    the <a href="http://www.gnu.org/licenses/old-licenses/gpl-2.0.html">GNU
	    General Public License, version 2</a>(GPLv2). For more
	    information follow the Link and/or see the file <tt>COPYING</tt> in
	    the LIVE source code directory.
	  </p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>New features in LIVE <?php echo $status->vers_number; ?></div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <?php include ("features_0-3-0.inc"); ?>
	  <p>
	    All past feature additions or changes of LIVE can be found on
	    the
	    <a href="news.php"><?php echo
	    $menu->urls["news.php"];?> archive page</a>.
	  </p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Direct download of LIVE <?php echo $status->vers_number; ?></div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <strong>Version <?php echo $status->vers_number; ?></strong>
            <ul>
	      <li>
		Download: <?php echo "<a href=\"http://live.vdr-developer.org/downloads/vdr-live-" . $status->vers_number . ".tar.gz\">" . "http://live.vdr-developer.org/downloads/vdr-live-" . $status->vers_number . ".tar.gz</a>"; ?>
	        <p>
		  <strong>IMPORTANT:</strong> please allways have a look at the <tt>README</tt> file in the LIVE source code directory!
		</p>
	      </li>
            </ul>
	  </p>
	  <p>
	    <strong>Installation prerequisites:</strong>
	    <?php include ("install_0-3-0.inc"); ?>
	  </p>
          <p>
	    Different versions of LIVE can be downloaded from the
	    <a href="download.php"><?php echo
	    $menu->urls["download.php"]; ?> page</a>.
	  </p>
        </div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<a name="links"></a>
	<div class="boxheader"><div><div>Links</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
          <p>
            <strong>Web:</strong>
            <ul>
	      <li><a href="http://projects.vdr-developer.org/projects/plg-live">Project Tracker</a></li>
	      <li><a href="http://projects.vdr-developer.org/git/vdr-plugin-live.git/">LIVE Git</a></li>
              <li><a href="http://www.tvdr.de">Video Disc Recorder - VDR</a></li>
              <li><a href="http://winni.vdr-developer.org/epgsearch/index.html">EPGSearch Plugin</a></li>
              <br />
              <li><a href="http://www.vdr-portal.de">VDR-Portal (mostly german)</a></li>
              <li><a href="http://www.vdr-wiki.de">VDR-Wiki (mostly german)</a></li>
            </ul>
          </p>
          <p>
            <strong>Other:</strong>
            <ul>
              <li>Additional contact information can be found at the top of the README file in the LIVE source distribution.</li>
            </ul>
          </p>
	</div>
      </div>

    </div>
  </body>
</html>
