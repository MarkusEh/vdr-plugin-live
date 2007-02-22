/*
 * Javascript functions for the status update box.
 * This file needs 'ajax.js' to be included on the pages.
 */

var vst_reload = true;
var vst_timer;
var vst_boxId = null;
var vst_url = null;

function LiveStatusAjaxRequest(url, containerid)
{
	if (vst_url == null)
		vst_url = url;
	if (vst_boxId == null)
		vst_boxId = containerid;

	var status = new LiveAjaxCall("text", url);
	status.oncomplete = function()
		{
			document.getElementById(containerid).innerHTML = this.xml.responseText;
			if (vst_reload)
				vst_timer = window.setTimeout("LiveStatusAjaxRequest('" + url + "', '" + containerid + "')", 1000);
		}
	status.onerror = function(message)
		{
			vst_reload = false;
			document.getElementById(containerid).innerHTML =
			'<div class="statuscontent>' +
				'<div class="st_header">' +
					'<div class="caption">ERROR</div>' +
					'<div class="now">&nbsp;<img id="statusReloadBtn" src="reload.png" alt="toggle reload on and off" onclick="LiveStatusToggleUpdate()" />&nbsp;ERROR</div>' +
				'</div>' +
				'<div class="st_content">' +
					'<div class="name">' + message + '</div>' +
				'</div>' +
			'</div>';
		}
	status.request("", "");
}

function LiveStatusToggleUpdate()
{
	if (vst_reload) {
		vst_reload = false;
		if (vst_timer != null)
			window.clearTimeout(vst_timer);
	}
	else {
		vst_reload = true;
		LiveStatusAjaxRequest(vst_url, vst_boxId);
	}
	var img = document.getElementById('statusReloadBtn');
	if (img != null) {
		// change image according to state.
		img.src = vst_reload ? 'stop.png' : 'reload.png';
	}
}
