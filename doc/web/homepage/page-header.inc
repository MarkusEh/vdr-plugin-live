      <a href="index.php"><img src="../img/logo.png" alt="VDR Live!" class="logo" /></a>
      <div id="infobox">
	<div class="statuscontent">
	  <div class="st_header">
	    <div id="infobox_caption" class="caption"><?php echo $status->title; ?></div>
	    <div id="infobox_timenow" class="now"><?php echo date($status->datefmt); ?></div>
	  </div>
	  <div class="st_content">
	    <div id="infobox_name" class="name"><?php echo $status->vers_string; ?>:</div>
	    <div id="infobox_duration" class="duration"><?php echo $status->vers_number; ?></div>
	  </div>
	  <div class="st_controls">
	    <div id="infobox_channel_buttons">
	      <a id="infobox_prevchan" href="vdr_request/switch_channel?param=" title="Sender zurück"><img src="../img/one_downarrow.png" alt=""></img></a>
	      <a id="infobox_nextchan" href="vdr_request/switch_channel?param=" title="Sender vor"><img src="../img/one_uparrow.png" alt=""></img></a>
	    </div>
	  </div>
	</div>
      </div>
