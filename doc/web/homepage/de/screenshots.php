<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR Live - Screenshots</title>
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
	<div class="boxheader"><div><div>Screenshots</div></div></div>
	<div class="screenshots">
	  <?php
	     if (isset($_GET["img"])) {
	     	$screenshots->FullImage($_GET["img"]);
	     }
	     else {
	        $screenshots->AllImg();
             }
	  ?>
<!--
          <a href="img/whatson.jpg"><img src="img/whatson_thumb.jpg" alt="whats on"></img></a>
          <a href="img/schedule.jpg"><img src="img/schedule_thumb.jpg" alt="whats on"></img></a>
          <a href="img/timers.jpg"><img src="img/timers_thumb.jpg" alt="whats on"></img></a>
          <a href="img/searchepg.jpg"><img src="img/searchepg_thumb.jpg" alt="whats on"></img></a>
          <a href="img/edit_searchtimer.jpg"><img src="img/edit_searchtimer_thumb.jpg" alt="whats on"></img></a>
          <a href="img/recordings.jpg"><img src="img/recordings_thumb.jpg" alt="whats on"></img></a>
          <a href="img/remote.jpg"><img src="img/remote_thumb.jpg" alt="whats on"></img></a>
          <a href="img/setup.jpg"><img src="img/setup_thumb.jpg" alt="whats on"></img></a>
-->
	</div>
      </div>

    </div>
  </body>
</html>
