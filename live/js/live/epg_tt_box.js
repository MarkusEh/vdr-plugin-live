/*
 * This is part of the live VDR plugin. See COPYING for license information.
 */

function classSetValue(element_id, value) {
  if (document && document.getElementById(element_id) ) {
    document.getElementById(element_id).setAttribute("class", value);
  }
}
function removeAttributeHidden(element_id) {
  if (document && document.getElementById(element_id) ) {
    document.getElementById(element_id).removeAttribute("hidden");
    classSetValue(element_id + "_m", "active");
  }
}
function addAttributeHidden(element_id) {
  if (document && document.getElementById(element_id) ) {
    document.getElementById(element_id).hidden = true;
    classSetValue(element_id + "_m", "inactive");
  }
}
function displayOn3Off(element_id_on, element_id_off1, element_id_off2, element_id_off3) {
 removeAttributeHidden(element_id_on);
 addAttributeHidden(element_id_off1);
 addAttributeHidden(element_id_off2);
 addAttributeHidden(element_id_off3);
} 
function displayEPG(boxId) {
  displayOn3Off(boxId + "_epg", boxId + "_scraper", boxId + "_actors", boxId + "_artwork");
  removeAttributeHidden(boxId + "_epg_image");
}
function displayScraper(boxId) {
  displayOn3Off(boxId + "_scraper", boxId + "_epg", boxId + "_actors", boxId + "_artwork");
  removeAttributeHidden(boxId + "_epg_image");
}
function displayActors(boxId) {
  displayOn3Off(boxId + "_actors", boxId + "_epg", boxId + "_scraper", boxId + "_artwork");
  addAttributeHidden(boxId + "_epg_image");
}
function displayArtwork(boxId) {
  displayOn3Off(boxId + "_artwork", boxId + "_actors", boxId + "_epg", boxId + "_scraper");
  addAttributeHidden(boxId + "_epg_image");
}
