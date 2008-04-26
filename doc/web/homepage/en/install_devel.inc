<?php
   require_once ("../live-sw-configs.php");

   $selectedConfig = 'devel';
?>
        <div class="tab">
          <table class="listing" style="margin-bottom: 15px; width: 100%" cellspacing="0" cellpadding="0">
            <tr class="description">
              <td><div class="withmargin">Software</div></td>
              <td><div class="withmargin">minimal version</div></td>
              <td><div class="withmargin">recommended</div></td>
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
              <td colspan="4"><div class="withmargin">Optional:</div></td>
            </tr>
	  <?php
	  foreach ($LiveSWConfigs[$selectedConfig]->Optional() as $sc) {
	    SWConfTableRow($sc);
	  }
	  ?>
            <tr class="spacer">
              <td colspan="4"></td>
            </tr>
            <tr class="description">
              <td colspan="4"><div class="withmargin">Used VDR Plugins:</div></td>
            </tr>
	  <?php
	  foreach ($LiveSWConfigs[$selectedConfig]->Plugins() as $sc) {
	    SWConfTableRow($sc);
	  }
	  ?>
          </table>
        </div>
	<p>
	  Tntnet provides the basic web server function for LIVE.
	  Cxxtools is needed for Tntnet.<br />*) Boost provides the
	  TR1 C++ extensions. These are only needed when compiled with
	  GCC &lt; 4.1. Later GCC versions provide the TR1 extensions
	  natively.
	</p>
