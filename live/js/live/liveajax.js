/*
 * This is part of the live VDR plugin. See COPYING for license information.
 *
 * Javascript function for quick AJAX requests.
 * This file needs mootools.js to be included on the pages.
 */

function LiveSimpleAjaxRequest(url, param, value)
{
  var req = new Ajax(url, {
      method : 'post'
    }).request(param + '=' + value + '&async=1');
};
