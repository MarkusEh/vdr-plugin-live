<?php
   require_once ("../live-sw-configs.php");

   $selectedConfig = '0.1.0';
?>
	<div class="tab">
          <table class="listing" style="margin-bottom: 15px; width: 100%" cellspacing="0" cellpadding="0">
            <tr class="description">
	      <td><div class="withmargin">Software</div></td>
	      <td><div class="withmargin">Mindestversion</div></td>
	      <td><div class="withmargin">Empfohlen</div></td>
	      <td><div class="withmargin">URL</div></td>
	    </tr>
	  <?php
	  foreach ($LiveSWConfigs[$selectedConfig]->Required() as $sc) {
	    SWConfTableRow($sc);
	  }
	  ?>
            <tr class="spacer">
	      <td colspan="4"></td>
	    </tr>
            <tr class="description">
	      <td colspan="4"><div class="withmargin">Verwendete VDR Plugins:</div></td>
	    </tr>
	  <?php
	  foreach ($LiveSWConfigs[$selectedConfig]->Plugins() as $sc) {
	    SWConfTableRow($sc);
	  }
	  ?>
          </table>
	</div>

	<p>
          Tntnet stellt grundlegende Webserver-Funktionen für LIVE zur
          Verfügung.  Cxxtools wird von Tntnet benötigt.<br />
	  Boost stellt wichtige Datenstrukturen. Gegenwärtig benötigen
	  wir das Gesamtpaket, demnächst könnte es aber eine
	  abgespeckte Version, die nur das benötigte bereitstellt, von
	  uns geben.
	</p>
