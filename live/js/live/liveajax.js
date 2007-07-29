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
