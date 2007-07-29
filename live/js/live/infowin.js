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
		  destroyOnHide: false,
		  className: 'info',
		  wm: false, // overide default window manager.
		  draggable: true,
		  resizable: true,
		  buttonimg: 'transparent.png',
		  bodyselect: 'div.content',
		  titleselect: 'div.caption',
		  classSuffix: '-win',
		  idSuffix: '-id',
		  offsets: {'x': -16, 'y': -16}
	  },

	  initialize: function(id, options){
			this.setOptions(options);
			this.wm = this.options.wm || InfoWin.$wm;
			this.winFrame = $(id + this.options.classSuffix + this.options.idSuffix);
			if (!$defined(this.winFrame)){
				this.buildFrame(id);
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
			// header of window: upper shadows, corners title and controls
			var top = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-top'
				}).inject(this.winFrame);
			if (this.options.draggable) this.winFrame.makeDraggable({'handle': top});
			top = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(top);
			this.titleBox = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-t'
				}).inject(top);

			this.buttonBox = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-b'
				}).inject(top);
			var cls = new Element('div', {
					'class': 'close'
				}).inject(this.buttonBox);
			cls.addEvent('click', function(event){
					var event = new Event(event);
					event.stop();
					return this.hide();
				}.bind(this));
			cls = new Element('img', {
					'src': this.options.buttonimg,
					'alt': 'close',
					'width': '16px',
					'height': '16px'
				}).inject(cls);

			// body of window: user content.
			var bdy = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-body'
				}).inject(this.winFrame);
			bdy = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(bdy);
			this.winBody = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-s'
				}).inject(bdy);

			// bottom border of window: lower shadows and corners, optional
			// resize handle.
			var bot = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-bot'
				}).inject(this.winFrame);
			bot = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(bot);

			if (this.options.resizable) {
				this.winFrame.makeResizable({'handle': bot});
			}

			if (!this.fillTitle(id)) {
				// todo: add generic title
			}
			return this.fillBody(id);
		},

	  buildFrame: function(id){
			this.winFrame = new Element('div', {
					'id': id + this.options.classSuffix + this.options.idSuffix,
					'class': this.options.className + this.options.classSuffix,
					'styles': {
						'position': 'absolute',
						'top': '0',
						'left': '0'
					}
				});
		},

	  show: function(event){
			this.position(event);
			this.fireEvent('onShow', [this.winFrame]);
			this.wm.raise(this);
			if (this.options.timeout)
				this.timer = this.hide.delay(this.options.timeout, this);
			return false;
		},

	  hide: function(){
			this.fireEvent('onHide', [this.winFrame]);
			if (this.options.destroyOnHide) {
				this.wm.unregister(this);
				for (var z in this) this[z] = null;
				this.destroyed = true;
			}
			else {
				this.wm.bury(this);
			}
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
		  closedContainer: 'infowin-closed',
		  openedContainer: 'infowin-opened',
		  onRegister: Class.empty,
		  onUnregister: Class.empty,
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
			this.fireEvent('onRegister', [infoWin]);
			infoWin.winFrame.addEvent('click', function(){
					this.raise(infoWin);
				}.bind(this));
			infoWin.winFrame.inject(this.closedWins);
		},

	  unregister: function(infoWin){
			this.fireEvent('onUnregister', [infoWin]);
			infoWin.winFrame.remove();
		},

	  raise: function(infoWin){
			this.fireEvent('onRaise', [infoWin]);
			infoWin.winFrame.inject(this.openedWins);
		},

	  bury: function(infoWin){
			this.fireEvent('onBury', [infoWin]);
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
				this.addEvent('onError', function(){
						this.hide.delay(1000, this);
					}.bind(this));
				var ajax = new Ajax(url, {
					  update: this.ajaxResponse,
					  onComplete: function(text, xmldoc){
							this.fillTitle(id);
							this.fillBody(id);
							this.ajaxResponse.remove();
						}.bind(this),
					  onFailure: function(transport){
							this.titleBox.setHTML(this.options.errorMsg);
							this.fireEvent('onError', [id, url]);
						}.bind(this)
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


/*
Class: Infowin.Notifier

	Creates a notification popup that disappears automaticaly.
	Usefull for a confirmation message after a AJAX action request.
 */

InfoWin.Notifier = InfoWin.extend({
	  options: {
		  timeout: 2500,
		  destroyOnHide: true,
		  className: 'ok',
		  classSuffix: '-info',
		  message: '',
		  offsets: {'x': 16, 'y': 16}
	  },

	  initialize: function(id, options){
			this.parent(id, options);
		},

	  build: function(id){
			/* top border of hint */
			var top = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-top'
				}).inject(this.winFrame);
			top = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(top);

			/* body of tip: some helper divs and content */
			var bdy = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-body'
				}).inject(this.winFrame);
			bdy = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(bdy);
			this.winBody = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-s'
				}).inject(bdy);

			/* bottom border of tip */
			var bot = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-bot'
				}).inject(this.winFrame);
			bot = new Element('div', {
					'class': this.options.className + this.options.classSuffix + '-c'
				}).inject(bot);

			return this.fillBody(id);
		},

	  fillBody: function(id){
			this.winFrame.setStyle('position', 'fixed');
			this.winBody.empty().setHTML(this.options.message);
			return true;
		},

	  position: function(event){
			var prop = {'x': 'left', 'y': 'top'};
			for (var z in prop) {
				var pos = this.options.offsets[z];
				this.winFrame.setStyle(prop[z], pos);
			}
		}
	});

