<?php
   require_once ("../live-sw-configs.php");

   $selectedConfig = '0.3.0';
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
              <td colspan="4"><div class="withmargin">optional:</div></td>
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
              <td colspan="4"><div class="withmargin">used VDR plugins:</div></td>
            </tr>
          <?php
          foreach ($LiveSWConfigs[$selectedConfig]->Plugins() as $sc) {
            SWConfTableRow($sc);
          }
          ?>
          </table>
        </div>

        <p>
          Tntnet provides basic web server functionality for LIVE.
          Cxxtools is required by Tntnet.<br />PCRE is used for the
          search with regular expressions on the recordings page.<br
          />*) Boost provides TR1 C++ extensions and is required only
          when compiled with GCC &lt; 4.1.
        </p>
