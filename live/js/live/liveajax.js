/*
 * Javascript function for quick ajax requests.
 * This file needs mootools.js to be included on the pages.
 */

function LiveSimpleAjaxRequest(url, param, value)
{
	var req = new Ajax(url, {
		  method : 'post'
		}).request(param + '=' + value + '&async=1');
};
function delete_recording(recid)
{
  var req = new XMLHttpRequest();
  req.open('POST', encodeURI('delete_recording.html?param=' + recid + '&async=1'), false);
  req.overrideMimeType("text/xml");
  req.send();
  var ret_object = new Object();
  ret_object.success = false;
  if (!req.responseXML) {
    ret_object.error = "invalid xml, no responseXML";
    return ret_object;
  }
  var response_array = req.responseXML.getElementsByTagName("response");
  if (response_array.length != 1) {
    ret_object.error = "invalid xml, no response tag or several response tags";
    return ret_object;
  }
  var response_child_nodes = response_array[0].childNodes;
  if (response_child_nodes.length != 1) {
    ret_object.error = "invalid xml, no child of response tag or several childs of response tag";
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue == "success") {
    ret_object.success = true;
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue != "error") {
    ret_object.error = "invalid xml, response node value " + response_child_nodes[0].nodeValue + " unknown";
    return ret_object;
  }


  var error_array = req.responseXML.getElementsByTagName("error");
  if (error_array.length != 1) {
    ret_object.error = "invalid xml, no error tag or several error tags";
    return ret_object;
  }
  var error_child_nodes = error_array[0].childNodes;
  if (error_child_nodes.length != 1) {
    ret_object.error = "invalid xml, no child of error tag or several childs of error tag";
    return ret_object;
  }
  ret_object.error = error_child_nodes[0].nodeValue;
};
function delete_rec_back(recid, history_num_back)
{
  var ret_object = delete_recording(recid);
  if (!ret_object.success) alert (ret_object.error);
  history.go(-history_num_back);
};
