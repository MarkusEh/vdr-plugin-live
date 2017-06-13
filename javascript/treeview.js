// ---------------------------------------------
// --- Name:    Easy DHTML Treeview           --
// --- Author:  D.D. de Kerf                  --
// --- Adapted: Dieter Hametner		      --
// --- Version: 0.2          Date: 13-6-2001  --
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

function updateCookieOnExpand( id ){
	var openNodes = readCookie( cookieNameRec );
	if (openNodes == null || openNodes == "")
		openNodes = id;
	else
		openNodes += "," + id;
	createCookie( cookieNameRec, openNodes, 14 );
}

function updateCookieOnCollapse( id ){
	var openNodes = readCookie( cookieNameRec );
	if (openNodes != null)
		openNodes = openNodes.split(",");
	else
		openNodes = [];
	for (var z=0; z<openNodes.length; z++){
		if (openNodes[z] === sibling.id){
			openNodes.splice(z,1);
			break;
		}
	}
	openNodes = openNodes.join(",");
	createCookie( cookieNameRec, openNodes, 14 );
}

function openNodesOnPageLoad(){
	var openNodes = readCookie( cookieNameRec );
	if (openNodes != null && openNodes !== "")
		openNodes = openNodes.split(",.,");
	else
		openNodes = [];
	for (var z=0; z<openNodes.length; z++){
		var ul = document.getElementById(openNodes[z]);
		if (ul){
			ul.style.display = 'block';
			//var imgChild = findChildNode(ul.parentNode, "recording_imgs");
			var imgChild = ul.parentNode.children[0].children[0];
			if (imgChild != null)
				setImages(imgChild, "img/minus.png", "img/folder_open.png");
		}
	}
}

var cookieNameRec = "VDR-Live-Recordings-Tree-Open-Nodes";

window.addEvent('domready', function(){
	openNodesOnPageLoad();
}); 


//The following cookie functions are taken from http://www.quirksmode.org/js/cookies.html

function createCookie(name,value,days) {
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function readCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function eraseCookie(name) {
	createCookie(name,"",-1);
}
