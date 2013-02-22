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
	    <strong>Version 0.3.0</strong>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.3.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <strong>Installation prerequisites:</strong>
	    <?php include ("install_0-3-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Development version:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <p>
	    <strong>Current GIT snapshot</strong>

	    <br/>Since LIVE is at present in maintainance mode there
	    are no major changes to be afraid of. So you can regard
	    the developer version as stable too.

            <ul>
	      <li>
		GIT repository: <a href="http://projects.vdr-developer.org/git/vdr-plugin-live.git/">http://projects.vdr-developer.org/git/vdr-plugin-live.git/</a>
	        <p>
		  <strong>IMPORTANT:</strong> when using this version
		  please always check the README file in the LIVE
		  source directory.
		</p>
	      </li>
            </ul>
	  </p>
	  <p>
	    <strong>Installation prerequisites:</strong>
	    <?php include ("install_devel.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Old versions:</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
          <p>
            <strong>Version 0.2.0</strong>
            <ul>
              <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.2.0.tar.gz</a></li>
            </ul>
          </p>
          <p>
            <strong>Installation prerequisites:</strong>
            <?php include ("install_0-2-0.inc"); ?>
          </p>
          <p><hr/></p>
	  <p>
	    <strong>Version 0.1.0</strong>
            <ul>
	      <li>Download: <a href="http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz">http://live.vdr-developer.org/downloads/vdr-live-0.1.0.tar.gz</a></li>
            </ul>
	  </p>
	  <p>
	    <strong>Installation prerequisites:</strong>
	    <?php include ("install_0-1-0.inc"); ?>
	  </p>
          <p></p>
	</div>
      </div>
    </div>
  </body>
</html>
