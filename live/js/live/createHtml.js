/*
 * This is part of the live VDR plugin. See COPYING for license information.
 *
 * Helper functions to create HTML.
 */


function addEncodeHtml(s, str) {
  s.a += str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/[\n\r]/g, '<br/>');
}

function addIfWide(text) {
 if (!window.matchMedia("(max-width: 600px)").matches) document.write(text);
}
function addIfSmall(text) {
 if (window.matchMedia("(max-width: 600px)").matches) document.write(text);
}

function truncateOnWordIdx(str, limit) {
  var b = str.indexOf('&lt;br/&gt;')
  if (b >= 0 && b<= limit) return b
  var c = str.indexOf('<br/>')
  if (c >= 0 && c<= limit) return c
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
  return str.slice(0,l) + ' ...'
}

function addTime(s, time) {
// add time in seconds, in format minutes:ss
  var d_sec = time%60
  s.a += String((time-d_sec)/60)
  s.a += ':'
  var d_sec_ld = d_sec%10
  s.a += String.fromCharCode(48+(d_sec-d_sec_ld)/10,48+(d_sec%10))
}

function addScraperImageTitle(s, image, pt, title, seasonEpisode, runtime, date) {
// pt: "pt" if m_s_image.width <= m_s_image.height, otherwise= ""
// seasonEpisode: e.g. 3E8    (we will add the missing S ...)
  s.a += '<div class=\"thumb\"><img loading="lazy" data-src=\"';
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
        s.a += '<br/>S'
        s.a += seasonEpisode
      }
      if (runtime.length != 0) {
        s.a += '<br/>'
        s.a += runtime
      }
      if (date.length != 0) {
        s.a += '<br/>'
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
    s.a += ' ...'
    return
  }
  var ll = truncateOnWordIdx(text, liml)
  s.a += '<span class="hidden-xs">'
  s.a += text.slice(ls, ll)
  s.a += '</span>'
  if (text.length == ll) s.a += '<span class="display-xs"> ...</span>'
  else s.a += ' ...'
}

function add2ndLine(s, shortText, description) {
// second line (title / short text). Truncate, use description, ...
  s.a += '<span class="short">'
  if (shortText.length != 0) {
    addTruncMedia(s, shortText, 50, 80)
  } else {
    if (description.length == 0) s.a += '&nbsp;'
    else addTruncMedia(s, description, 50, 80)
  }
  s.a += '</span>'
}

// do not html encode title! will be html encoded here
function addColEventRec(s, times, eventprefix, eventid, title, folder, shortText, description) {
// col with times, channel, name, short text
  s.a += '<div class="withmargin"><div class="margin-bottom display-xs"><span class="normal-font">'
  s.a += times
  s.a += '</span></div>'
// sec&third line: Link to event, event title, short text
  addEventRec(s, eventprefix, eventid, '&history_num_back=1', title, folder, shortText, description)
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
 *   url: URL to the page triggering the execution of the function
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
    ret_object.error = "invalid XML, no responseXML";
    return ret_object;
  }
  var response_array = req_responseXML.getElementsByTagName("response");
  if (response_array.length != 1) {
    ret_object.error = "invalid XML, no response tag or several response tags";
    return ret_object;
  }
  var response_child_nodes = response_array[0].childNodes;
  if (response_child_nodes.length != 1) {
    ret_object.error = "invalid XML, no child of response tag or several children of response tag";
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue == "1") {
    ret_object.success = true;
    return ret_object;
  }
  if (response_child_nodes[0].nodeValue != "0") {
    ret_object.error = "invalid XML, response node value " + response_child_nodes[0].nodeValue + " unknown";
    return ret_object;
  }

  var error_array = req_responseXML.getElementsByTagName("error");
  if (error_array.length != 1) {
    ret_object.error = "invalid XML, no error tag or several error tags";
    return ret_object;
  }
  var error_child_nodes = error_array[0].childNodes;
  if (error_child_nodes.length != 1) {
    ret_object.error = "invalid XML, no child of error tag or several children of error tag";
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

// events[day][0]: day
// events[day][1][ev][0][]: event data
// events[day][1][ev][1][]: if available: existing recording data
//
function addEventList(s, col_span, events) {
  s.a += '<table class="listing" cellspacing="0" cellpadding="0">'
  for (let day=0; day < events.length; day++) {
    if (day != 0) {
      s.a += '<tr class="spacer"><td colspan='
      s.a += col_span
      s.a += '/></tr>\n'
    }
    s.a += '<tr class="head"><td colspan='
    s.a += col_span
    s.a += '><div class="boxheader"><div><div>'
    s.a += events[day][0]
    s.a += '</div></div></div></td></tr>'
    for (let event_=0; event_ < events[day][1].length; event_++) {
      if (events[day][1][event_].length == 1 && event_ == events[day][1].length-1) {
        addEvent(s, 1, events[day][1][event_][0])    // bottom
      } else {
        addEvent(s, 0, events[day][1][event_][0])
      }
      if (events[day][1][event_].length == 2) {
// existing recording
        if (event_ == events[day][1].length-1) {
          bottomrow = 'bottomrow'
        } else {
          bottomrow = ''
        }
// note: data is written as needed by existingRecordingString
// which differs soewhat from existingRecordingSR
        existingRecordingSR(s, col_span-2, bottomrow, events[day][1][event_][1][2], events[day][1][event_][1][0], events[day][1][event_][1][1], events[day][1][event_][1][3], events[day][1][event_][1][4], events[day][1][event_][1][5], events[day][1][event_][1][6], events[day][1][event_][1][7], events[day][1][event_][1][8], events[day][1][event_][1][9], events[day][1][event_][1][10], events[day][1][event_][1][11], events[day][1][event_][1][12], events[day][1][event_][1][13], events[day][1][event_][1][14], events[day][1][event_][1][15], events[day][1][event_][1][16], events[day][1][event_][1][17], events[day][1][event_][1][18], events[day][1][event_][1][19], events[day][1][event_][1][21])
      }
    }
  }
  s.a += '</table>\n'
}
function addEventListString(col_span, events) {
  const s = Object.create(null)
  s.a = ""
  addEventList(s, col_span, events)
  return s.a
}
