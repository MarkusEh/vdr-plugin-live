// ---------------------------------------------
// --- Name:    Easy DHTML Treeview           --
// --- Author:  D.D. de Kerf                  --
// --- Adapted: Markus Ehrnsperger            --
// --- Adapted: Jasmin Jessich                --
// --- Adapted: hepi (via patch)              --
// --- Adapted: Dieter Hametner               --
// --- Version: 0.3          Date: 14-6-2017  --
// ---------------------------------------------

function set_folder_closed(rec_list) {
  const fldr_hash = rec_list.id;
  uncheck_all(rec_list);
  updateCookieOnCollapse(fldr_hash);

  const folder_symbol_element=document.getElementById('fs_'+fldr_hash);
//folder_symbol_element.setAttribute("onClick", '');
  set_icons_closed(document.getElementById('pm_'+fldr_hash), folder_symbol_element);

//document.getElementById('ca_'+fldr_hash).disabled = true;
  rec_list.style.display = 'none';
}
async function set_folder_open(rec_list) {
// return true in case of a dom change
  const fldr_hash = rec_list.id;
  updateCookieOnExpand(fldr_hash);

  var dom_changed = false;

  const rec_id = rec_ids[fldr_hash];
  if (rec_id != null && rec_id.length >= 3 && rec_id[1] != 2) {
    for (var rec of rec_id[2]) {
      const cb=document.getElementById("cb_"+rec);
      if (cb == null) {
        dom_changed = true;
        break;
      }
    }
    if (dom_changed) {
      rec_list.insertAdjacentHTML("beforeend", await rec_string_d_a(rec_id));
    }
    rec_id[1] = 2;
  }

  const folder_symbol_element=document.getElementById('fs_'+fldr_hash);
//folder_symbol_element.setAttribute( "onClick", 'javascript: SetChecked("'+fldr_hash+'")');
  set_icons_open(document.getElementById('pm_'+fldr_hash), folder_symbol_element);

//document.getElementById('ca_'+fldr_hash).disabled = false;
  rec_list.style.display = 'revert-layer';
  return dom_changed;
}

function SetCheckboxValues(fldr_hash, value) {
//document.getElementById('ca_'+fldr_hash).checked = value;

  if (rec_ids[fldr_hash] == null || rec_ids[fldr_hash].length < 3) return;
  for (var item of rec_ids[fldr_hash][2]) {
    const cb=document.getElementById("cb_"+item);
    if (cb != null) cb.checked=value;
  }
}
function uncheck_all(rec_list)
{
  SetCheckboxValues(rec_list.id, false);
  for (var recordingNode of rec_list.getElementsByClassName("recordingslist") ) {
    SetCheckboxValues(recordingNode.id, false);
  }
}

async function click_folder_line (e, fldr_hash) {
//alert('click_folder_line, currentTarget='+e.currentTarget.id+' target='+e.target.id+' fldr_hash='+fldr_hash);
//alert('click_folder_line, target='+e.target.id+' fldr_hash='+fldr_hash);

  const rec_list = document.getElementById(fldr_hash);
  if (rec_list == null) return;

  if (rec_list.style.display == 'none') {
    if (await set_folder_open(rec_list) ) {
      if (typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
      imgLoad();
    }
  } else {
    if ('fs_'+fldr_hash == e.target.id) {
// click on open folder symbol -> select all recordings in this folder
      SetCheckboxValues(fldr_hash, true);
    } else {
// Collapse the branch if it IS visible
      set_folder_closed(rec_list);
    }
  }
}
async function Toggle(node, fldr_hash) {
// Unfold the branch if it isn't visible
  alert("Toggle, fldr_hash="+fldr_hash);
  const rec_list = document.getElementById(fldr_hash);
  if (rec_list == null) return;

  if (rec_list.style.display == 'none') {
    if (await set_folder_open(rec_list) ) {
      if (typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
      imgLoad();
    }
  } else {
// Collapse the branch if it IS visible
    set_folder_closed(rec_list);
  }
}
function ToggleChecked(node, fldr_hash) {
// If unchecked,   check all items.
// If   checked, uncheck all items.
  const rec_list = document.getElementById(fldr_hash);
  if (rec_list == null) return;

  if (rec_list.style.display == 'none') return;
  SetCheckboxValues(fldr_hash, node.checked);
}

function SetChecked(fldr_hash) {
// check all items.
  const rec_list = document.getElementById(fldr_hash);
  if (rec_list == null) return;

  if (rec_list.style.display == 'none') return;
  SetCheckboxValues(fldr_hash, true);
}
function updateCookieOnExpand( id )
{
  var openNodes = readCookie(cookieNameRec);
  if (openNodes == null || openNodes == "")
    openNodes = id;
  else {
    for (var openNode of openNodes.split(",")) {
      if (openNode === id) return;
    }
    openNodes += "," + id;
  }
  createCookie(cookieNameRec, openNodes, 14);
}

function updateCookieOnCollapse(id)
{
let openNodes = readCookie(cookieNameRec);
if (openNodes != null)
  openNodes = openNodes.split(",");
else
  openNodes = [];
for (var z=0; z<openNodes.length; z++) {
  if (openNodes[z] === id){
    openNodes.splice(z,1);
    break;
  }
}
openNodes = openNodes.join(",");
createCookie(cookieNameRec, openNodes, 14);
}

async function openNodesOnPageLoad()
{
  let openNodes = readCookie(cookieNameRec);
  if (openNodes != null && openNodes !== "")
    openNodes = openNodes.split(",");
  else
    openNodes = [];
  let domChanges = 0;
  for (var openNode of openNodes) {
    const rec_id = rec_ids[openNode];
//  if (!openNode) continue;  // otherwise throws error for level 0
    const rec_list = document.getElementById(openNode);
    if (rec_list == null) continue;
    if (await set_folder_open(rec_list) ) domChanges = 1;
  }
  if (domChanges == 1 && typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
  for (var openNode of openNodes) {
    const rec_id = rec_ids[openNode];
  }
  imgLoad();
}

function getElementsByNodeNameClassName(elementd, nodeName, className) {
// example: getElementsByNodeNameClassName(window.document, 'DIV', 'test')
const classElements = elementd.getElementsByClassName(className);
  return Array.prototype.filter.call(
    classElements, (classElement) => classElement.nodeName === nodeName,
  );
}

function filterRecordings(filter, currentSort, currentFlat, recycle_bin)
{
  window.location.href = "recordings.html?sort=" + currentSort + "&flat=" + currentFlat + "&filter=" + encodeURIComponent(filter.value) + "&recycle_bin=" + recycle_bin;
}
function deletedRecordings(recycle_bin, currentSort, currentFilter)
{
  if (recycle_bin.checked) {
    window.location.href = "recordings.html?sort=" + currentSort + "&flat=true&filter=" + currentFilter + "&recycle_bin=1";
  } else {
    window.location.href = "recordings.html?sort=" + currentSort + "&flat=true&filter=" + currentFilter + "&recycle_bin=0";
  }
}

async function ExpandAll()
{
  var domChanges = 0;
  recordingNodes = getElementsByNodeNameClassName(window.document, 'UL', "recordingslist");
  for (idx = 0; idx < recordingNodes.length; idx++) {
    if (recordingNodes[idx].parentNode.className != 'recordings') {
      if (await set_folder_open(recordingNodes[idx]) ) domChanges = 1;
    }
  }
  if (domChanges == 1 && typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
  if (domChanges == 1) imgLoad();
}
function CollapseAll()
{
  recordingNodes = getElementsByNodeNameClassName(window.document, 'UL', "recordingslist");
  for (idx = 0; idx < recordingNodes.length; idx++) {
    if (recordingNodes[idx].parentNode.className != 'recordings') {
      set_folder_closed(recordingNodes[idx]);
    }
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
  document.cookie = name+"="+value+expires+";SameSite=Lax; path=/";
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
