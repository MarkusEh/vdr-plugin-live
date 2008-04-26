<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR Live - Screenshots</title>
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

      <div style="width: 840px; margin-top: 15px">
	<div class="boxheader"><div><div>
	  <?php
	     if (isset($_GET["img"])) {
	     	echo "Screenshot: " . $screenshots->ImageDescr($_GET["img"], "Unbekanntes Bild");
	     }
	     else {
	     	echo "Screenshots: Overview";
	     }
	  ?>
	</div></div></div>
	<div class="screenshots">
	  <?php
	     if (isset($_GET["img"])) {
	     	$screenshots->FullImage($_GET["img"]);
	     }
	     else {
	        $screenshots->AllImg();
             }
	  ?>
	</div>
      </div>

    </div>
  </body>
</html>
