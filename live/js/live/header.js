/*
 * This is part of the live VDR plugin. See COPYING for license information.
 */

function adjustHeader()
{
  // exchange body top margin by equally-sized header-padding element
  var padding = document.getElementById( "padding" );
  if( padding.style.height == "" )
  {
    var bodyStyles = ( window.getComputedStyle )? getComputedStyle( document.body, null ) : document.body.currentStyle;
    padding.style.height = bodyStyles.marginTop;
    padding.style.backgroundColor = ( window.bgColor )? window.bgColor : "white";
    padding.style.display = "";
    document.body.style.marginTop = "0px";
  }
  // expand underlay to header's height
  var header = document.getElementById( "header" );
  var underlay = document.getElementById( "underlay" );
  underlay.style.height = header.offsetHeight+ "px";
  underlay.style.display = "";
  header.style.position = "fixed";
  return;
}

