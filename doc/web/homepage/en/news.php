<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
  <head>
    <title>VDR Live - News</title>
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
	<div class="boxheader"><div><div>Features of LIVE 0.2.0</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <?php include ("features_0-2-0.inc"); ?>
	</div>
      </div>

      <div style="width: 800px; margin-top: 15px">
	<div class="boxheader"><div><div>Features of LIVE 0.1.0</div></div></div>
	<div style="border: 1px solid black; padding: 0px 10px">
	  <?php include ("features_0-1-0.inc"); ?>
	</div>
      </div>

    </div>
  </body>
</html>
