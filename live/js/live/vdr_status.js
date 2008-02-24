/*
 * Javascript functions for the status update box.
 * This file needs mootools.js to be included on the pages.
 */

var LiveVdrInfo = Ajax.extend({

	  options: {
		  autoCancel: true
	  },

	  initialize: function(url, boxId)
	  {
		  this.parent(url, null);
		  this.addEvent('onComplete', this.showInfo);
		  this.addEvent('onFailure', this.reportError);

		  this.boxId = boxId;
		  this.reload = true;
		  this.timer = null;
	  },

	  request: function(update)
	  {
		  this.parent({update: update ? "1" : "0"});
	  },

	  showInfo: function(text, xmldoc)
	  {
		  try {
			  this.selectInfoElems(xmldoc);

			  this.setEpgInfo(xmldoc);

			  this.setInfoMessage(xmldoc);

			  this.setUpdate(xmldoc);
		  }
		  catch (e) {
			  this.reportError(null);
		  }
	  },

	  reportError: function(transport)
	  {
		  this.setTextContent('caption', 'ERROR');
		  var message;
		  if (transport != null) {
			  message = $("__infobox_request_err").firstChild.nodeValue;
		  }
		  else {
			  message = $("__infobox_update_err").firstChild.nodeValue;
		  }
		  this.setTextContent('name', message);
	  },

	  // private function to switch visibility of controls.
	  selectInfoElems: function(xmldoc)
	  {
		  var infoType = xmldoc.getElementsByTagName('type').item(0);

		  var channel = $(this.boxId + '_channel_buttons');
		  var playback = $(this.boxId + '_recording_buttons');

		  if (infoType.firstChild.nodeValue != "channel") {
			  channel.style.display = 'none';
			  playback.style.display = 'block';
			  this.setTextContent('pause', infoType.firstChild.nodeValue);
			  this.setTextContent('play', infoType.firstChild.nodeValue);
			  this.setTextContent('rwd', infoType.firstChild.nodeValue);
			  this.setTextContent('ffw', infoType.firstChild.nodeValue);
			  this.setTextContent('stop', infoType.firstChild.nodeValue);
		  }
		  else {
			  playback.style.display = 'none';
			  channel.style.display = 'block';
		  }
	  },

	  // private function to activate the info message display if the
	  // corresponding element is found in the current page.
	  setInfoMessage: function(xmldoc)
	  {
		  var info = xmldoc.getElementsByTagName('info').item(0);
		  if (! $defined(info))
			  return;

		  var messagebar = $('messagebar');
		  if (! $defined(messagebar))
			  return;

		  var message = xmldoc.getElementsByTagName('message').item(0);
		  var url = xmldoc.getElementsByTagName('url').item(0);

		  if (message.firstChild.nodeValue != "") {
			  $('mbmessage').setText(message.firstChild.nodeValue);
			  if ($defined(url.firstChild)) {
				  $('mbdelimiter').removeClass('notpresent');
				  $('mbreact').setProperty('href', url.firstChild.nodeValue);
			  }
			  else {
				  $('mbdelimiter').addClass('notpresent');
				  $('mbreact').addClass('notpresent');
			  }
			  messagebar.removeClass('notpresent');
		  }
	  },

	  // private function to display information from epg info.
	  setEpgInfo: function(xmldoc)
	  {
		  var epgInfo = xmldoc.getElementsByTagName('epginfo').item(0);

		  for (var i = 0; i < epgInfo.childNodes.length; i++) {
			  var node = epgInfo.childNodes.item(i);
			  if (node.nodeType == 1) {
				  var textContent = "";
				  if (node.firstChild != null)
					  textContent = node.firstChild.nodeValue;
				  this.setTextContent(node.nodeName, textContent);
			  }
		  }
	  },

	  // private function to update text contents.
	  setTextContent: function(nodeName, textContent)
	  {
		  var docNode = $(this.boxId + '_' + nodeName);
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
					  // docNode.href = "javascript:LiveSimpleAjaxRequest('switch_channel.xml', 'param', '" + textContent + "');";
					  docNode.href = "vdr_request/switch_channel?param=" + textContent;
					  docNode.style.visibility = "visible";
				  }
				  else {
					  docNode.style.visibility = "hidden";
				  }
				  break;
			  }
			  case "pause":
			  case "play":
			  case "rwd":
 			  case "ffw":
			  case "stop":
			  {
				  if (textContent != "") {
					  // docNode.href = "javascript:LiveSimpleAjaxRequest('" + nodeName + "_recording.xml', 'param', '" + textContent + "');";
					  docNode.href = "vdr_request/" + nodeName + "_recording?param=" + textContent;
					  docNode.style.visibility = "visible";
				  }
				  else {
					  docNode.style.visibility = "hidden";
				  }
				  break;
			  }
			  default:
				  break;
			  }
		  }
	  },

	  // private function to determine update status and to trigger
	  // the next update.
	  setUpdate: function(xmldoc)
	  {
		  /* check if we still need to update the status */
		  var upd = xmldoc.getElementsByTagName('update').item(0);
		  var rel = (upd.firstChild.nodeValue == "1");

		  if (rel != this.reload) {
			  this.reload = rel;
			  var img = $('statusReloadBtn');
			  if (img != null) {
				  // change image according to state.
				  img.src = this.reload ? 'img/stop_update.png' : 'img/reload.png';
			  }
		  }
		  if (this.reload)
			  this.timer = this.request.delay(1000, this, true);
	  },

	  toggleUpdate: function()
	  {
		  if (this.reload) {
			  if (this.timer != null) {
				  this.timer = $clear(this.timer);
			  }
		  }
		  this.request(!this.reload);
	  },

	  pageFinished: function()
	  {
		  if (this.reload) {
			  if (this.timer != null) {
				  this.timer = $clear(this.timer);
			  }
		  }
		  this.cancel();
	  }
});
