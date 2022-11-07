// ---------------------------------------------
// --- Name:    Easy DHTML Treeview           --
// --- Author:  D.D. de Kerf                  --
// --- Adapted: Jasmin Jessich                --
// --- Adapted: hepi (via patch)              --
// --- Adapted: Dieter Hametner               --
// --- Version: 0.3          Date: 14-6-2017  --
// ---------------------------------------------

function findSibling(node, name)
{
	while ((node.nextSibling.nodeType != Node.ELEMENT_NODE)
	       || (node.nextSibling.nodeName != name)) {
		node = node.nextSibling;
	}
	if (node.nextSibling.nodeName == name)
		return node.nextSibling;

	return null;
}

function findChildNode(node, className)
{
	for (idx = 0; idx < node.childNodes.length; idx++) {
		n = node.childNodes.item(idx);
		if (n.nodeType == Node.ELEMENT_NODE) {
			attr = n.getAttributeNode("class");
			if ((attr != null) && (attr.nodeValue == className)) {
				return n;
			}
		}
	}
	return null;
}

function findImageNode(node, className)
{
	for (idx = 0; idx < node.childNodes.length; idx++) {
		n = node.childNodes.item(idx);
		if ((n.nodeType == Node.ELEMENT_NODE) && (n.nodeName == "IMG")) {
			attr = n.getAttributeNode("class");
			if ((attr != null) && (attr.nodeValue == className)) {
				return n;
			}
		}
	}
	return null;
}

function setImages(node, expand, folder)
{
	// Change the image (if there is an image)
	if (node.childNodes.length > 0)
	{
		expandNode = findImageNode(node, "recording_expander");
		if (expandNode != null)
			expandNode.src = expand;
		folderNode = findImageNode(node, "recording_folder");
		if (folderNode != null)
			folderNode.src = folder;
	}
}

function Toggle(node)
{
	// Unfold the branch if it isn't visible
	sibling = findSibling(node, "UL");
	if (sibling == null)
		return;

	imgChild = findChildNode(node, "recording_imgs");
	if (sibling.style.display == 'none')
	{
		if (imgChild != null)
			setImages(imgChild, "img/minus.png", "img/folder_open.png");
		sibling.style.display = 'block';
		updateCookieOnExpand( sibling.id );
	}
	// Collapse the branch if it IS visible
	else
	{
		updateCookieOnCollapse( sibling.id );
		if (imgChild != null)
			setImages(imgChild, "img/plus.png", "img/folder_closed.png");
		sibling.style.display = 'none';
	}
}
function Toggle2(node, node_id)
{
	// Unfold the branch if it isn't visible
	sibling = findSibling(node, "UL");
	if (sibling == null)
		return;

	imgChild = findChildNode(node, "recording_imgs");
	if (sibling.style.display == 'none')
	{
		if (imgChild != null)
			setImages(imgChild, "img/minus.png", "img/folder_open.png");
                  if (rec_ids[node_id] != null && rec_ids[node_id].length > 0) {
		    sibling.insertAdjacentHTML("beforeend", rec_string(rec_ids[node_id]));
                    rec_ids[node_id] = [];
                    if (typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
		  }
		sibling.style.display = 'block';
		updateCookieOnExpand( sibling.id );
	}
	// Collapse the branch if it IS visible
	else
	{
		updateCookieOnCollapse( sibling.id );
		if (imgChild != null)
			setImages(imgChild, "img/plus.png", "img/folder_closed.png");
		sibling.style.display = 'none';
	}
}

function updateCookieOnExpand( id )
{
	var openNodes = readCookie( cookieNameRec );
	if (openNodes == null || openNodes == "")
		openNodes = id;
	else
		openNodes += "," + id;
	createCookie( cookieNameRec, openNodes, 14 );
}

function updateCookieOnCollapse( id )
{
	var openNodes = readCookie( cookieNameRec );
	if (openNodes != null)
		openNodes = openNodes.split(",");
	else
		openNodes = [];
	for (var z=0; z<openNodes.length; z++){
		if (openNodes[z] === id){
			openNodes.splice(z,1);
			break;
		}
	}
	openNodes = openNodes.join(",");
	createCookie( cookieNameRec, openNodes, 14 );
}

function imgLoad() {
var imgDefer = document.getElementsByTagName('img');
  for (var i = 0; i < imgDefer.length; i++) {
    if (imgDefer[i].getAttribute('data-src')) {
      imgDefer[i].setAttribute('src',imgDefer[i].getAttribute('data-src'));
    }
  }
}

function openNodesOnPageLoad()
{
	var openNodes = readCookie( cookieNameRec );
  	var domChanges = 0;
	if (openNodes != null && openNodes !== "")
		openNodes = openNodes.split(",");
	else
		openNodes = [];
	for (var z=0; z<openNodes.length; z++){
		var ul = document.getElementById(openNodes[z]);
		if (ul){
			ul.style.display = 'block';
                  	if (rec_ids[openNodes[z]] != null && rec_ids[openNodes[z]].length > 0) {
                          ul.insertAdjacentHTML("beforeend", rec_string_d(rec_ids[openNodes[z]]));
//                          ul.innerHTML += rec_string(rec_ids[openNodes[z]]);
//                          ul.set('html', ul.innerHTML + rec_string(rec_ids[openNodes[z]]))  // does not work with mootools 1.11
	                  rec_ids[openNodes[z]] = [];
			  domChanges = 1;
		  	}
			var imgChild = ul.parentNode.children[0].children[0];
			if (imgChild != null)
				setImages(imgChild, "img/minus.png", "img/folder_open.png");
		}
	}
	if (domChanges == 1 && typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
	imgLoad();
}

function getElementsByNodeNameClassName(elementd, nodeName, className) {
// example: getElementsByNodeNameClassName(window.document, 'DIV', 'test')
  const classElements = elementd.getElementsByClassName(className);
  return Array.prototype.filter.call(
    classElements, (classElement) => classElement.nodeName === nodeName,
  );
}

function filterRecordings(filter, currentSort, currentFlat)
{
  window.location.href = "recordings.html?sort=" + currentSort + "&flat=" + currentFlat + "&filter=" + encodeURIComponent(filter.value);
}
function ExpandAll()
{
  var openNodes = "";
  var domChanges = 0;
//			recordingNodes = window.document.getElementsBySelector("ul.recordingslist");
  recordingNodes = getElementsByNodeNameClassName(window.document, 'UL', "recordingslist");
  for (idx = 0; idx < recordingNodes.length; idx++) {
    if (recordingNodes[idx].parentNode.className != 'recordings') {
      recordingNodes[idx].style.display = 'block';
      openNodes += recordingNodes[idx].id + ","; 
      if (rec_ids[recordingNodes[idx].id] != null && rec_ids[recordingNodes[idx].id].length > 0) {
        recordingNodes[idx].insertAdjacentHTML("beforeend", rec_string_d(rec_ids[recordingNodes[idx].id]));
        rec_ids[recordingNodes[idx].id] = [];
        domChanges = 1;
      }
    }
  }
  if (domChanges == 1 && typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
  if (domChanges == 1) imgLoad();
  expandNodes = getElementsByNodeNameClassName(window.document, 'IMG', 'recording_expander');
  for (idx = 0; idx < expandNodes.length; idx++) {
    expandNodes[idx].src = "img/minus.png";
  }
  folderNodes = getElementsByNodeNameClassName(window.document, 'IMG', 'recording_folder');
  for (idx = 0; idx < folderNodes.length; idx++) {
    folderNodes[idx].src = "img/folder_open.png";
  }
  createCookie( cookieNameRec, openNodes, 14 );
}
function CollapseAll()
{
  recordingNodes = getElementsByNodeNameClassName(window.document, 'UL', "recordingslist");
  for (idx = 0; idx < recordingNodes.length; idx++) {
    if (recordingNodes[idx].parentNode.className != 'recordings') {
      recordingNodes[idx].style.display = 'none';
    }
  }
  expandNodes = getElementsByNodeNameClassName(window.document, 'IMG', 'recording_expander');
  for (idx = 0; idx < expandNodes.length; idx++) {
    expandNodes[idx].src = "img/plus.png";
  }
  folderNodes = getElementsByNodeNameClassName(window.document, 'IMG', 'recording_folder');
  for (idx = 0; idx < folderNodes.length; idx++) {
    folderNodes[idx].src = "img/folder_closed.png";
  }
  eraseCookie( cookieNameRec );
}

var cookieNameRec = "VDR-Live-Recordings-Tree-Open-Nodes";

document.addEventListener("DOMContentLoaded", function()
{
	openNodesOnPageLoad();
}); 

//The following cookie functions are taken from http://www.quirksmode.org/js/cookies.html

function createCookie(name,value,days)
{
        if (value.length > 1000) return; // too large cookies result in too large http headers
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function readCookie(name)
{
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function eraseCookie(name)
{
	createCookie(name,"",-1);
}
