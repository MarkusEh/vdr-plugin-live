/*
 * Javascript functions for the status update box.
 * This file needs 'ajax.js' to be included on the pages.
 */

var vst_reload = true;
var vst_timer;
var vst_boxId = null;
var vst_url = null;

function LiveStatusRequest(url, containerid)
{
	if (vst_url == null)
	{
		vst_url = url;
	}
	if (vst_boxId == null)
		vst_boxId = containerid;

	var status = new LiveAjaxCall("xml", url);
	status.oncomplete = function()
		{
			try {
				LiveStatusShowInfo(this.xml.responseXML, containerid);
			}
			catch (e) {
				LiveStatusReportError(e.message, containerid);
			}
			if (vst_reload)
				vst_timer = window.setTimeout("LiveStatusRequest('" + url + "', '" + containerid + "')", 1000);
		}
	status.onerror = function(message)
		{
			LiveStatusToggleUpdate();
 			LiveStatusReportError(message, containerid);
		}
	status.request("update", vst_reload ? "1" : "0");
}

function LiveStatusShowInfo(xmldoc, containerId)
{
	var epgInfo = xmldoc.getElementsByTagName('epginfo').item(0);

	for (var i = 0; i < epgInfo.childNodes.length; i++) {
		var node = epgInfo.childNodes.item(i);
		if (node.nodeType == 1) {
			var textContent = "";
			if (node.firstChild != null)
				textContent = node.firstChild.nodeValue;
			LiveStatusSetTextContent(containerId, node.nodeName, textContent);
		}
	}
}

function LiveStatusReportError(message, containerId)
{
	LiveStatusSetTextContent(containerId, 'caption', 'ERROR');
	LiveStatusSetTextContent(containerId, 'name', message);
}

function LiveStatusSetTextContent(containerId, nodeName, textContent)
{
	var docNode = document.getElementById(containerId + '_' + nodeName);
	if (docNode != null) {
		switch (nodeName) {
			case "caption":
			case "timenow":
			case "name":
			case "duration":
			{
				if (docNode.innerHTML != textContent)
					docNode.innerHTML = textContent;
				break;
			}
			case "elapsed":
			{
				var width = textContent + "px";
				if (docNode.style.width != width)
					docNode.style.width = width;
				break;
			}
			case "nextchan":
			case "prevchan":
			{
				if (textContent != "") {
					docNode.href = "javascript:LiveSimpleAjaxRequest('switch_channel.xml', 'param', '" + textContent + "');";
					docNode.style.visibility = "visible";
				}
				else {
					docNode.style.visibility = "hidden";
				}
			}
			default:
				break;
		}
	}
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
		LiveStatusRequest(vst_url, vst_boxId);
	}
	var img = document.getElementById('statusReloadBtn');
	if (img != null) {
		// change image according to state.
		img.src = vst_reload ? 'stop.png' : 'reload.png';
	}
}
