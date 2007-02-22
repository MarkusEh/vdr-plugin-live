function LiveAjaxCall(mode, url)
{
	var xml = null;
	if (window.XMLHttpRequest) {
		xml = new XMLHttpRequest();
		if (("xml" == mode) && xml.overrideMimeType)
			xml.overrideMimeType('text/xml');
	} else if (window.ActiveXObject) {
		try {
			xml = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
				xml = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}

	this.url = url;
	this.xml = xml;

	this.onerror = function(message) {};
	this.oncomplete = function() {};

	this.request = function(param, value)
	{
		var url = this.url;
		if (param != "") {
			var url = this.url+'?'+param+"="+value;
		}
		var obj = this;
		this.xml.onreadystatechange = function() { obj.readystatechanged(); }
		this.xml.open('GET', url, true);
		this.xml.send(null);
	};

	this.readystatechanged = function()
	{
		try {
			if (this.xml.readyState == 4) {
				if (this.xml.status == 200) {
					if ("xml" == mode) {
						var xmldoc = xml.responseXML;
						var result = Number(xmldoc.getElementsByTagName('response').item(0).firstChild.data);
						if (!result) {
							var error = xmldoc.getElementsByTagName('error').item(0).firstChild.data;
							this.onerror(error);
						} else {
							this.oncomplete();
						}
					} else {
						this.oncomplete();
					}
				} else {
					this.onerror('Invocation of webservice "'+this.url+'" failed with http status code '+this.xml.status);
				}
			}
		} catch (e) {
			this.onerror('Invocation of webservice "'+this.url+'" failed with exception: '+e.message);
		}
	};
}

function LiveSimpleAjaxRequest(url, param, value)
{
	var xml = new LiveAjaxCall("xml", url);
	xml.onerror = function(message) { alert(message); }
	xml.request(param, value);
};
