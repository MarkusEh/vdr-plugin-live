/*
 * This is part of the live vdr plugin. See COPYING for license information.
 *
 * VLC class.
 *
 * This class adds convenience methods to a embeded vlc media player
 * object and allows control of the player and provides an event interface.
 */

/*
Class: VLC
	A VLC plugin wrapper.

Arguments:
	id - The id of the embedded vlc plugin.
	options - optional arguments, helping to tweak this class to your html.

Options:
	autoresize - if true, the player will be resized with the browser window.
	controlsContainer - the id of the DOM element that contains the controls.
	controls - an array describing which controls are provided by the html
			   page. Current supported types: play, mute, screen,
			   close.  for each type your page wants to provide an
			   inline object is expected with with the following
			   properties:
			   type - one of the types above to map to the class internal
			          functions for that type.
			   id - the DOM id of the control.
			   classes - on: class to add when toggled on.
			           - off: class to add when toggled off.

Events:
	ontoggle - event fired after the toggling of one property took place.
 */

var VLC = new Class({
	  options: {
		  autoresize: true,
		  controlsContainer: "vlcControls",
		  // select by type the actions should be performed by a
		  // instanciated object of class VLC. Possible types you find
		  // below in the 'actions' definition.
		  controls: [
			{ type: 'play',
			  id: "TogglePlay",
			  classes: { on: "red", off: "green" }},
			{ type: 'mute',
			  id: "ToggleMute",
			  classes: { on: "green", off: "red" }},
			{ type: 'screen',
			  id: "FullScreen",
			  classes: { on: "blue", off: "blue" }},
			{ type: 'close',
			  id: "Close",
			  classes: { on: "yellow", off: "yellow" }},
		  ],
		  offset: 5,
		  playRecording: false
	  },

	  initialize: function(id, options){
			this.setOptions(options);
			this.id = id;
			window.addEvent('domready', this.playerSetup.bind(this));
		},

	  playerSetup: function(){
			this.vlc = $(this.id);
			this.newVlcApi = (this.vlc.VersionInfo != null);
			// add here new actions these class might support:
			var actions = {
			  play: { check: this.isPlaying, toggle: this.togglePlay },
			  mute: { check: this.isMuted, toggle: this.toggleMute },
			  screen: { check: Class.empty, toggle: this.toggleScreen },
			  close: { check: Class.empty, toggle: this.close }};
			$each(this.options.controls, function(item, idx){
					var elem = $(item.id);
					if (elem && actions[item.type]) {
						item.fns = actions[item.type];
					}
				}, this);

			this.setStates();
			var idx = 0;
			$each(this.options.controls, function(item){
					if (item.fns && item.fns.toggle)
						$(item.id).addEvent('click', function (event, item){
								var toggle = item.fns.toggle.bind(this);
								var check = item.fns.check.bind(this);
								toggle();
								this.fireEvent('toggle', [item.id, check()]);
							}.bindWithEvent(this, item));
				}, this);
			if (this.options.autoresize) {
				window.addEvent('resize', this.playerResize.bind(this));
			}
		},

	  enableDeinterlace: function(){
			if (this.newVlcApi) {
				this.vlc.video.deinterlace.enable("yadif");
			}
		},

	  disableDeinterlace: function(){
			if (this.newVlcApi) {
				this.vlc.video.deinterlace.disable();
			}
		},

	  playerResize: function(el){
			var winwidth = window.getWidth();
			var winheight = window.getHeight();
			winheight -= $(this.options.controlsContainer).getSize().size.y;
			winheight -= this.options.offset;
			this.vlc.setStyle('width', winwidth);
			this.vlc.setStyle('height', winheight);
		},

	  isPlaying: function(){
			if (this.newVlcApi)
				return this.vlc.playlist && this.vlc.playlist.isPlaying;
			else
				return this.vlc.isplaying();
		},

	  isMuted: function(){
			if (this.newVlcApi)
				return this.vlc.audio && this.vlc.audio.mute;
			else {
				var res = this.vlc.get_volume();
				return 0 == res;
			}
		},

	  togglePlay: function(){
			if (this.newVlcApi)
				if (!this.options.playRecording)
					this.vlc.playlist.togglePause();
				else {
					if (this.vlc.playlist.isPlaying) {
						clearTimeout(this.deint);
						this.disableDeinterlace();
						this.vlc.playlist.stop();
					else {
						this.vlc.playlist.play();
						this.deint = setTimeout(this.enableDeinterlace, 500);
					}
				}
			else {
				if (this.isPlaying())
					this.vlc.stop();
				else
					this.vlc.play();
			}
			this.setStates();
		},

	  toggleMute: function(){
			if (this.newVlcApi)
				this.vlc.audio.toggleMute();
			else
				this.vlc.mute();
			this.setStates();
		},

	  toggleScreen: function(){
			if (this.newVlcApi)
				this.vlc.video.toggleFullscreen();
			else
				this.vlc.fullscreen();
			this.setStates();
		},

	  close: function(){
			window.close();
		},

	  setStates: function(){
			$each(this.options.controls, function(item, idx){
					if (item.fns && (Class.empty != item.fns.check)) {
						var fn = item.fns.check.bind(this);
						if (fn()) {
							$(item.id).removeClass(item.classes.off);
							$(item.id).addClass(item.classes.on);
						}
						else {
							$(item.id).removeClass(item.classes.on);
							$(item.id).addClass(item.classes.off);
						}
					}
				}, this);
		}
	});

VLC.implement(new Events, new Options);
