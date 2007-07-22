/*
 * This is part of the live vdr plugin. See COPYING for license information.
 *
 * InfoWin.js
 *
 * InfoWin class, InfoWin.Manager class, InfoWin.Ajax class.
 *
 * Extension of mootools to display a popup window with some html
 * code.
 */

/*
Class: InfoWin
	Create an information window as overlay to current page.

Arguments:

Options:

Note:
	A window consists of a frame-element. This is the overall
	containing element used to control the display and size of the
	window. It is accesable through the 'winFrame' property.

	The InfoWin class provides the followin properties to fill the
	window with content:
		- titleBox: the element meant to place the title of the window into.
		- buttonBox: here the default window buttons are created. You might
			clear this and create your own kind of window controls.
		- winBody: this is where your window contents goes.
 */
var InfoWin = new Class({
	  options: {
		  timeout: 0,
		  onShow: Class.empty,
		  onHide: Class.empty,
		  onDomExtend: Class.empty,
		  className: 'info',
		  wm: false, // overide default window manager.
		  draggable: true,
		  resizable: true,
		  buttonimg: 'transparent.png',
		  bodyselect: 'div.content',
		  titleselect: 'div.caption',
		  idSuffix: '-win-id',
		  offsets: {'x': -16, 'y': -16}
	  },

	  initialize: function(id, options){
			this.setOptions(options);
			this.wm = this.options.wm || InfoWin.$wm;
			this.winFrame = $(id + this.options.idSuffix);
			if (!$defined(this.winFrame)){
				this.build(id);
				this.wm.register(this);
			}
		},

	  // internal: build new window element.
	  //
	  // build sets up a frame for a new InfoWin. The parent element
	  // of the window frame has the id '<id>-win-id'. The function
	  // must return true if the body of the InfoWin has been filled
	  // with the user data, false otherwise.
	  build: function(id){
			this.winFrame = new Element('div', {
					'id': id + this.options.idSuffix,
					'class': this.options.className + '-win',
					'styles': {
						'position': 'absolute',
						'top': '0',
						'left': '0'
					}
				});

			// header of window: upper shadows, corners title and controls
			var top = new Element('div', {
					'class': this.options.className + '-win-top'
				}).inject(this.winFrame);
			if (this.options.draggable) this.winFrame.makeDraggable({'handle': top});
			top = new Element('div', {
					'class': this.options.className + '-win-c'
				}).inject(top);
			this.titleBox = new Element('div', {
					'class': this.options.className + '-win-t'
				}).inject(top);

			this.buttonBox = new Element('div', {
					'class': this.options.className + '-win-b'
				}).inject(top);
			var cls = new Element('div', {
					'class': 'close'
				}).inject(this.buttonBox);
			var self = this;
			cls.addEvent('click', function(event){
					var event = new Event(event);
					event.stop();
					return self.hide();
				});
			cls = new Element('img', {
					'src': this.options.buttonimg,
					'alt': 'close',
					'width': '16px',
					'height': '16px'
				}).inject(cls);

			// body of window: user content.
			var bdy = new Element('div', {
					'class': this.options.className + '-win-body'
				}).inject(this.winFrame);
			bdy = new Element('div', {
					'class': this.options.className + '-win-c'
				}).inject(bdy);
			this.winBody = new Element('div', {
					'class': this.options.className + '-win-s'
				}).inject(bdy);

			// bottom border of window: lower shadows and corners, optional
			// resize handle.
			var bot = new Element('div', {
					'class': this.options.className + '-win-bot'
				}).inject(this.winFrame);
			bot = new Element('div', {
					'class': this.options.className + '-win-c'
				}).inject(bot);

			if (this.options.resizable) {
				this.winFrame.makeResizable({'handle': bot});
			}

			if (!this.fillTitle(id)) {
				// todo: add generic title
			}
			return this.fillBody(id);
		},

	  show: function(event){
			if (this.options.timeout)
				this.timer = this.hide.delay(this.options.timeout, this);
			this.position(event);
			this.fireEvent('onShow', [this.winFrame]);
			this.wm.raise(this);
			return false;
		},

	  hide: function(){
			this.fireEvent('onHide', [this.winFrame]);
			this.wm.bury(this);
			return false;
		},

	  fillBody: function(id){
			var bodyElems = $$('#'+ id + ' ' + this.options.bodyselect);
			if ($defined(bodyElems) && bodyElems.length > 0) {
				this.winBody.empty();
				this.fireEvent('onDomExtend', [id, bodyElems]);
				this.winBody.adopt(bodyElems);
				return true;
			}
			return false;
		},

	  fillTitle: function(id){
			var titleElems = $$('#' + id + ' ' + this.options.titleselect);
			if ($defined(titleElems) && titleElems.length > 0) {
				this.titleBox.empty().adopt(titleElems);
				return true;
			}
			return false;
		},

	  position: function(event){
			var prop = {'x': 'left', 'y': 'top'};
			for (var z in prop) {
				var pos = event.page[z] + this.options.offsets[z];
				this.winFrame.setStyle(prop[z], pos);
			}
		}
	});

InfoWin.implement(new Events, new Options);

/*
Class: InfoWin.Manager
	Provide an container and events for the created info win
	instances.  Closed info-wins are preserved in a hidden dom element
	and used again if a window with a closed id is openend again.
*/
InfoWin.Manager = new Class({
	  options: {
		  zIndex: 100,
		  closedContainer: 'infowin-closed',
		  openedContainer: 'infowin-opened',
		  onRegister: Class.empty,
		  onRaise: Class.empty,
		  onBury: Class.empty
	  },

	  initialize: function(options){
			this.setOptions(options);
			// initialize properties this.closedWins and this.openedWins:
			['closed', 'opened'].each(function(kind){
					var wins = kind + 'Wins';
					var opts = this.options[kind + 'Container'];
					this[wins] = $(opts);
					if (!$defined(this[wins])){
						this[wins] = new Element('div', {
								'id': opts,
								'styles' : {
									'display' : (kind == 'closed') ? 'none' : 'block'
								}
							});
						this[wins].inject(document.body);
					}
				}, this);
		},

	  register: function(infoWin){
			var self = this;
			this.fireEvent('onRegister', [infoWin]);
			infoWin.winFrame.addEvent('click', function(){
					self.raise(infoWin);
				});
			infoWin.winFrame.inject(this.closedWins);
		},

	  raise: function(infoWin){
			this.fireEvent('onRaise', [infoWin]);
			infoWin.winFrame.remove();
			infoWin.winFrame.inject(this.openedWins);
		},

	  bury: function(infoWin){
			this.fireEvent('onBury', [infoWin]);
			infoWin.winFrame.remove();
			infoWin.winFrame.inject(this.closedWins);
		}
	});

InfoWin.Manager.implement(new Events, new Options);

InfoWin.$wm = null;
window.addEvent('domready', function(){
		InfoWin.$wm = new InfoWin.Manager();
	});

/*
Class: InfoWin.Ajax

	Use an instance of mootools Ajax class to asynchronously request
	the content of an info win.
*/
InfoWin.Ajax = InfoWin.extend({
	  options: {
		  loadingMsg: 'loading',
		  errorMsg: 'an error occured!',
		  onError: Class.empty
	  },

	  initialize: function(id, url, options){
			this.parent(id, options);
			if ($defined(this.ajaxResponse)) {
				var self = this;
				this.addEvent('onError', function(){
						self.hide.delay(1000, self);
					});
				var ajax = new Ajax(url, {
					  update: this.ajaxResponse,
					  onComplete: function(text, xmldoc){
							self.fillTitle(id);
							self.fillBody(id);
							self.ajaxResponse.empty();
						},
					  onFailure: function(transport){
							self.titleBox.setHTML(self.options.errorMsg);
							self.fireEvent('onError', [id, url]);
						}
					}).request('async=1');
			}
		},

	  // this function gets called when no previous instance for 'id'
	  // created a dom subtree for an infowin.
	  build: function(id){
			if (!this.parent(id)) {
				this.titleBox.setHTML(this.options.loadingMsg);
				this.ajaxResponse = new Element('div', {
						'styles' : {
							'display': 'none'
						}
					}).inject(this.winFrame);
			}
		}
	});

InfoWin.Ajax.implement(new Events, new Options);
