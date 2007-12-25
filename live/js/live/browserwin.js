/*
 * This is part of the live vdr plugin. See COPYING for license information.
 *
 * browserwin.js
 *
 * BrowserWin class, BrowserWin.Manager class
 *
 * Extension of mootools to create and manage browser windows.
 */

/*
Class: BrowserWin
	Create and browswer window pointing at an specific URL.

Arguments:

Options:

Note:
*/
var BrowserWin = new Class({
	  options: {
		  size: { width: 720, height: 640 },
		  toolbar: false,
		  location: false,
		  directories: false,
		  statusbar: false,
		  menubar: false,
		  scrollbar: false,
		  resizable: true,
		  wm: false // overide default window manager.
	  },

	  initialize: function(id, url, options){
			this.setOptions(options);
			this.id = id;
			this.wm = this.options.wm || BrowserWin.$wm;
			this.wm.register(this, url);
		},

	  create: function(url){
			winOpts =  "height=" + this.options.size.height;
			winOpts += ",width=" + this.options.size.width;
			winOpts += ",toolbar=" + this.options.toolbar;
			winOpts += ",location=" + this.options.toolbar;
			winOpts += ",directories=" + this.options.directories;
			winOpts += ",statusbar=" + this.options.statusbar;
			winOpts += ",menubar=" + this.options.menubar;
			winOpts += ",scrollbars=" + this.options.scrollbars;
			winOpts += ",resizable=" + this.options.resizable;
			if ($defined(this.options.top)) {
				winOpts += ",top=" + this.options.top;
			}
			if ($defined(this.options.left)) {
				winOpts += ",left=" + this.options.left;
			}
			this.$winRef = window.open(url, this.id, winOpts);
		},

	  close: function(){
			this.wm.unregister(this);
		}
	});

BrowserWin.implement(new Events, new Options);

BrowserWin.Manager = new Class({
	  options: {
		  onRegister: Class.empty,
		  onUnregister: Class.empty
	  },

	  initialize: function(options){
			this.setOptions(options);
			this.hashTab = new Hash();
		},

	  register: function(browserWin, url){
			this.unregister(browserWin);

			browserWin.create(url);
			this.hashTab.set(browserWin.id, browserWin);
			this.fireEvent('onRegister', [browserWin]);
		},

	  unregister: function(browserWin){
			if (this.hashTab.hasKey(browserWin.id)) {
				winRef = this.hashTab.get(browserWin.id);
				winRef.$winRef.close();
				this.fireEvent('onUnregister', [winRef]);
				this.hashTab.remove(browserWin.id);
			}
		}
	});

BrowserWin.Manager.implement(new Events, new Options);

BrowserWin.$wm = null;
window.addEvent('domready', function(){
		BrowserWin.$wm = new BrowserWin.Manager();
	});
