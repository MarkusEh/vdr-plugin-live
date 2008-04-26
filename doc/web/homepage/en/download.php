<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR LIVE - Downloads</title>
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
	<div class="boxheader"><div><div>Stable version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Version 0.2.0</b>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <b>Installation prerequisites:</b>
	    <?php include ("install_0-2-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Development version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Current CVS snapshot</b>
            <ul>
	      <li>
		Download: <a href="http://www.vdr-developer.org/cgi-bin/cvsweb.cgi/live/live.tar.gz?tarball=1">http://www.vdr-developer.org/cgi-bin/cvsweb.cgi/live/</a>
	        <p>
		  <strong>IMPORTANT:</strong> when using this version
		  please always check the README file in the LIVE
		  source directory.
		</p>
	      </li>
            </ul>
	  </p>
	  <p>
	    <b>Installation prerequisites:</b>
	    <?php include ("install_devel.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Old version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <b>Version 0.1.0</b>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <b>Installation prerequisites:</b>
	    <?php include ("install_0-1-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

    </div>
  </body>
</html>
