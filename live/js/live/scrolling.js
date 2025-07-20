/*
 * This is part of the live VDR plugin. See COPYING for license information.
 */

// save the scroll position of an element
function saveScrollPosition(id) {
  let element = document.getElementById(id);
  if (element) {
    let left = element.scrollLeft;
    let top  = element.scrollTop;
    if (left || top) {
      history.replaceState({ id: element.id, scrollLeft: left, scrollTop:  top }, "");
    }
  }
}

// restore a previously saved scroll position of an element
function restoreScrollPosition() {
  if (history.state) {
    let element = document.getElementById(history.state.id);
    let left = history.state.scrollLeft;
    let top  = history.state.scrollTop;
    if (element && (top || left)) {
      element.scrollTo(left, top);
    }
  }
}
