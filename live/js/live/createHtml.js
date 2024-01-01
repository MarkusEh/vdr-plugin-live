/*
 * This is part of the live vdr plugin. See COPYING for license information.
 *
 * helper functions to create html
 *
 */

function addIfWide(text) {
 if (!window.matchMedia("(max-width: 600px)").matches) document.write(text);
}
function addIfSmall(text) {
 if (window.matchMedia("(max-width: 600px)").matches) document.write(text);
}

function truncateOnWordIdx(str, limit) {
  var b = str.indexOf('&lt;br/&gt;')
  if (b >= 0 && b<= limit) return b
  var r = str.indexOf('\r')
  if (r >= 0 && r<= limit) return r
  var n = str.indexOf('\n')
  if (n >= 0 && n<= limit) return n
  if (str.length <= limit) return str.length
  var l = str.lastIndexOf(' ', limit);
  if (l <= 0) l = limit
  return l
}

function truncateOnWord(str, limit) {
  var l = truncateOnWordIdx(str, limit)
  if (str.length == l) return str
  return str.slice(0,l) + '...'
}

function addTime(s, time) {
// add time in seconds, in format minutes:ss
  var d_sec = time%60
  s.a += String((time-d_sec)/60)
  s.a += ':'
  var d_sec_ld = d_sec%10
  s.a += String.fromCharCode(48+(d_sec-d_sec_ld)/10,48+(d_sec%10))
}

function addScraperImageTitle(s, image, pt, title, seasonEpisode, runtime, date, lf) {
// pt: "pt" if m_s_image.width <= m_s_image.height, otherwise= ""
// seasonEpisode: e.g. 3E8    (we will add the missing S ...)
  s.a += '<div class=\"thumb\"><img data-src=\"';
  if (image.length != 0) {
    s.a += '/tvscraper/'
    s.a += image
    s.a += '\" class=\"thumb'
    s.a += pt
  } else s.a += 'img/transparent.png\" height=\"16px'
  if (title.length != 0 || date.length != 0) {
// scraper data available
    s.a += '\" title=\"'
    s.a += title
      if (seasonEpisode.length != 0) {
        s.a += lf
        s.a += 'S'
        s.a += seasonEpisode
      }
      if (runtime.length != 0) {
        s.a += lf
        s.a += runtime
      }
      if (date.length != 0) {
        s.a += lf
        s.a += date
      }
  }
  s.a += '\"/></div>'
}
function addTruncMedia(s, text, lims, liml) {
// lims: Text limit for small screens
// liml: Text limit for wide screens
  var ls = truncateOnWordIdx(text, lims)
  s.a += text.slice(0,ls)
  if (text.length == ls) return
  if (text.length <= lims) {
    s.a += '...'
    return
  }
  var ll = truncateOnWordIdx(text, liml)
  s.a += '<span class="hidden-xs">'
  s.a += text.slice(ls, ll)
  s.a += '</span>'
  if (text.length == ll) s.a += '<span class="display-xs">...</span>'
  else s.a += '...'
}

function add2ndLine(s, shortText, description) {
// second line (title / short text). Truncate, use decription, ...
  s.a += '<span class="short">'
  if (shortText.length != 0) {
    addTruncMedia(s, shortText, 50, 80)
  } else {
    if (description.length == 0) s.a += '&nbsp;'
    else addTruncMedia(s, description, 50, 80)
  }
  s.a += '</span>'
}

function addEventRec(s, eventprefix, eventid, title, folder, shortText, description, lf, cvd, sort, filter, flat, history_num_back) {
// eventprefix == 'recording_' or 'event_'
// lf: line feed
// cvs: tr("Click to view details.")
  s.a += '<a href="epginfo.html?epgid='
  s.a += eventprefix
  s.a += eventid
  s.a += '&sort='
  s.a += sort
  s.a += '&filter='
  s.a += encodeURIComponent(filter)
  s.a += '&flat='
  s.a += flat
  s.a += '&history_num_back='
  s.a += history_num_back
  s.a += '" class="apopup" title="'
  if (description.length != 0) {
    s.a += description
    s.a += lf
  }
  s.a += cvd
  s.a += '">'
  s.a += '<div class="margin-bottom bold-font">'
  s.a += title
  if (folder.length != 0) {
    s.a += '<span class="normal-font"> ('
    s.a += folder.replaceAll("~", "~<wbr>")
    s.a += ')</span>'
  }
  s.a += '</div>'                                                                                                                                 
  add2ndLine(s, shortText, description)
  s.a += '</a>'
}

function addColEventRec(s, times, eventprefix, eventid, title, folder, shortText, description, lf, cvd) {
// col with times, channel, name, short text
  s.a += '<div class="withmargin"><div class="margin-bottom display-xs"><span class="normal-font">'
  s.a += times
  s.a += '</span></div>'
// sec&third line: Link to event, event title, short text
  addEventRec(s, eventprefix, eventid, title, folder, shortText, description, lf, cvd, '', '', '', 1)
  s.a += '</div>'
}

function injectHdSdIcon(elementId, sdhd, channelName, frameParams) {
  const s = Object.create(null);
  s.a = "";
  addHdSdIcon(s, sdhd, channelName, frameParams);
  document.getElementById(elementId).innerHTML = s.a;
  if (typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
}

function injectErrorHdSdIcon(elementId, numErrors, durationDeviation, sdhd, channelName, duration, numTsFiles, frameParams) {
  const s = Object.create(null);
  s.a = "";
  addErrorIcon(s, numErrors, durationDeviation, duration, numTsFiles);
  addHdSdIcon(s, sdhd, channelName, frameParams);
  document.getElementById(elementId).innerHTML = s.a;
  if (typeof liveEnhanced !== 'undefined') liveEnhanced.domReadySetup();
}

function imgLoad() {
var imgDefer = document.getElementsByTagName('img');
  for (var i = 0; i < imgDefer.length; i++) {
    if (imgDefer[i].getAttribute('data-src')) {
      imgDefer[i].setAttribute('src',imgDefer[i].getAttribute('data-src'));
    }
  }
}

function clearCheckboxes(form) {
// clearing checkboxes
  var inputs = form.getElementsByTagName('input');
  for (var i = 0; i<inputs.length; i++) {
    if (inputs[i].type == 'checkbox') {
        inputs[i].checked = false;
    }
  }
}
async function execute(url) {
/*
 * Input:
 *   Url: url to the page triggering the execution of the function
 *        this includes the parameters
 *        '&async=1' will be appended (which is required to get an XML response,
 *             actually we wait for the server response)
 * Output:
 *   error object (struct) with fields
 *               - bool   success
 *               - string error  (only if success == false). Human readable text
*/
  const response = await fetch(encodeURI(url + '&async=1'), {
    method: "GET",
    headers: {
     "Content-Type": "application/x-www-form-urlencoded",
    },
  });
  const req_responseXML = new window.DOMParser().parseFromString(await response.text(), "text/xml");
  var ret_object = new Object();
  ret_object.success = false;
  if (!req_responseXML) {
    ret_object.error = "invalid xml, no responseXML";
    return ret_object;
  }
  var response_array = req_responseXML.getElementsByTagName("response");
  if (response_array.length != 1) {
    ret_object.error = "invalid xml, no response tag or several response tags";
    return ret_object;
  }
  var response_child_nodes = response_array[0].childNodes;
  if (response_child_nodes.length != 1) {
    ret_object.error = "invalid xml, no child of response tag or several childs of response tag";
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue == "1") {
    ret_object.success = true;
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue != "0") {
    ret_object.error = "invalid xml, response node value " + response_child_nodes[0].nodeValue + " unknown";
    return ret_object;
  }

  var error_array = req_responseXML.getElementsByTagName("error");
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
  return ret_object;
}
async function delete_rec_back(recid, history_num_back)
{
  var ret_object = await execute('delete_recording.html?param=' + recid);
  if (!ret_object.success) alert (ret_object.error);
  history.go(-history_num_back);
}
function back_depending_referrer(back_epginfo, back_others) {
  if (document.referrer.indexOf("epginfo.html?") != -1) {
    history.go(-back_epginfo);
  } else {
    history.go(-back_others);
  }
}
function RecordingsSt(s, level, displayFolder, data) {
  var recs_param =  '';
  for (obj_i of data) {
    if (typeof recs[obj_i] === 'undefined') {
      if (recs_param.length == 0) {
        recs_param += 'r=';
      } else {
        recs_param += '&r=';
      }
      recs_param += obj_i;
    }
  }
  if (recs_param.length == 0) {
    RecordingsSt_int(s, level, displayFolder, data);
  } else {
    const request = new XMLHttpRequest();
    request.open("POST", "get_recordings.html", false);
    request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    request.send(recs_param);
    eval(request.response);
    RecordingsSt_int(s, level, displayFolder, data);
  }
}
async function RecordingsSt_a(s, level, displayFolder, data) {
  var recs_param =  '';
  for (obj_i of data) {
    if (typeof recs[obj_i] === 'undefined') {
//      recs_param += '&r=';
      recs_param += param_name_recs;
      recs_param += obj_i;
    }
  }
  if (recs_param.length == 0) {
    RecordingsSt_int(s, level, displayFolder, data);
  } else {
    var recs_param_a = 'vdr_start=';
    recs_param_a += vdr_start;
    recs_param_a += '&recordings_tree_creation=';
    recs_param_a += recordings_tree_creation;
    recs_param_a += recs_param;
    const response = await fetch("get_recordings.html", {
      method: "POST",
      headers: {
       "Content-Type": "application/x-www-form-urlencoded",
      },
      body: recs_param_a,
    });
    const new_recs = await response.text();
    eval(new_recs);
    if (vdr_restart) {
      location.reload();    
    } else {
      RecordingsSt_int(s, level, displayFolder, data);
    }
  }
}
async function rec_string_d_a(rec_ids) {
  const st = Object.create(null)
  st.a = ""
  await RecordingsSt_a(st, rec_ids[0], rec_ids[1], rec_ids[2])
  return st.a
}

function rec_string_d(rec_ids) {
  const st = Object.create(null)
  st.a = ""
  RecordingsSt_int(st, rec_ids[0], rec_ids[1], rec_ids[2])
  return st.a
}
