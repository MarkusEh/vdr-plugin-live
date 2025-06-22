/*
 * This is part of the live VDR plugin. See COPYING for license information.
 *
 * InfoWin.js
 *
 * InfoWin class, InfoWin.Manager class, InfoWin.Ajax class.
 *
 * Extension of mootools to display a popup window with some HTML
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
  window. It is accessible through the 'winFrame' property.

  The InfoWin class provides the following properties to fill the
  window with content:

    - titleBox:  The element containing the title of the window.
    - buttonBox: The default window buttons are created here.
    - winBody:   This is where the actual window contents goes.
    - resizeBox: The element acting as anchor for the resize handle.
                 If resize is supported by the browser, this element
                 will remain invisible.
 */
var InfoWin = new Class({
    options: {
      timeout: 0,
      onShow: Class.empty,
      onHide: Class.empty,
      onDomExtend: Class.empty,
      destroyOnHide: false,
      className: 'info',
      wm: false, // override default window manager.
      draggable: true,
      resizable: true,
      resizeImg: 'img/transparent.png',
      closeImg: 'img/icon_overlay_cross.png',
      pinImg: 'img/icon_overlay_pin.png',
      pinnedImg: 'img/icon_overlay_pinned.png',
      bodySelect: 'div.content',
      titleSelect: 'div.caption',
      classSuffix: '-win',
      idSuffix: '-id',
      offsets: {'x': 0, 'y': 0}
    },

    initialize: function(id, options){
      this.setOptions(options);
      this.wm = this.options.wm || InfoWin.$wm;
      winFrameId = id + this.options.classSuffix + this.options.idSuffix;
      this.css = {'selector': 'div#' + winFrameId + ' '};
      this.winFrame = $(winFrameId);
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
      if (this.options.draggable) {
        top.setStyle('cursor', 'grab');;
        this.winFrame.makeDraggable({'handle': top, 'onComplete': function()
          { // make sure the window is within the 'content' container;
            // as the 'content' element uses scrolling, we do not need
            // to check for overflow in scroll direction
            if (this.element.offsetLeft < 0) {
              this.element.style.left = '0px';
            }
            if (this.element.offsetTop < 0) {
              this.element.style.top = '0px';
            }
          }
        });
      }
      this.titleBox = new Element('div', {
          'class': this.options.className + this.options.classSuffix + '-title'
        }).inject(top);

      this.buttonBox = new Element('div', {
          'class': this.options.className + this.options.classSuffix + '-buttons'
        }).inject(top);
      this.pinButton = new Element('img', {
          'src': this.options.pinImg,
          'class': 'iconic button pin',
          'alt': 'pin'
        }).inject(this.buttonBox);
      this.pinButton.addEvent('click', function(event){
          var event = new Event(event);
          winFrameRect = this.winFrame.getBoundingClientRect();
          if (this.winFrame.style.position == 'fixed') {
            // floating coordinates refer to the 'content' element
            content = document.getElementById('content');
            contentRect = content.getBoundingClientRect();
            this.winFrame.style.position = "absolute";
            this.winFrame.style.left = (winFrameRect.left - contentRect.left + content.scrollLeft) + 'px';
            this.winFrame.style.top  = (winFrameRect.top  - contentRect.top  + content.scrollTop ) + 'px';
            this.pinButton.src = this.options.pinImg;
          } else {
            // fixed coordinates refer to the viewport
            this.winFrame.style.position = 'fixed';
            this.winFrame.style.left = winFrameRect.left + 'px';
            this.winFrame.style.top  = winFrameRect.top  + 'px';
            this.pinButton.src = this.options.pinnedImg;
          }
          event.stop();
          return false;
        }.bind(this));
      closeButton = new Element('img', {
          'src': this.options.closeImg,
          'class': 'iconic button close',
          'alt': 'close'
        }).inject(this.buttonBox);
      closeButton.addEvent('click', function(event){
          var event = new Event(event);
          event.stop();
          return this.hide();
        }.bind(this));

      // body of window: user content.
      this.winBody = new Element('div', {
          'class': this.options.className + this.options.classSuffix + '-body'
        }).inject(this.winFrame);

      // by default, we rely on the CSS 'resize' property to for resizing;
      // if unsupported, and as fall-back approach, we inject a distinct
      // resize element for the resize handle of the mootools.
      if (this.options.resizable) {
        var resizeBox = new Element('div', {
            'class': this.options.className + this.options.classSuffix + '-resize'
          }).inject(this.winFrame);
        var icon = new Element('img', {
            'src': this.options.resizeImg,
            'class': 'icon resize',
            'alt': 'resize'
          }).inject(resizeBox);
        this.winFrame.makeResizable({'handle': resizeBox});
      }

      if (!this.fillTitle(id)) {
        // todo: add generic title
      }
      return this.fillBody(id);
    },

    buildFrame: function(id){
      this.winFrame = new Element('div', {
          'id': id + this.options.classSuffix + this.options.idSuffix,
          'class': this.options.className + this.options.classSuffix + ' ' + id.replace(/^([A-Za-z]*)_.*$/, this.options.className + '-$1'),
          'styles': {
            'position': 'absolute',
            'top': '0',
            'left': '0'
          }
        });
    },

    show: function(event){
      // raise before determining the position, as we then have the true
      // window dimensions derived from CSS settings for rectification
      // (instead of just some magic constants)
      this.wm.raise(this);
      this.position(event);
      if (this.winFrame.style.position != 'fixed') {
        // floating coordinates refer to the 'content' element
        content = document.getElementById('content');
        contentRect = content.getBoundingClientRect();
        this.winFrame.style.position = "absolute";
        this.winFrame.style.left = (parseInt(this.winFrame.style.left) - contentRect.left + content.scrollLeft) + 'px';
        this.winFrame.style.top  = (parseInt(this.winFrame.style.top)  - contentRect.top  + content.scrollTop ) + 'px';
      }
      this.fireEvent('onShow', [this.winFrame]);
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
        if (this.winFrame.style.position == 'fixed') {
            // floating coordinates refer to the 'content' element
            content = document.getElementById('content');
            contentRect = content.getBoundingClientRect();
            this.winFrame.style.position = "absolute";
            this.winFrame.style.left = (winFrameRect.left - contentRect.left + content.scrollLeft) + 'px';
            this.winFrame.style.top  = (winFrameRect.top  - contentRect.top  + content.scrollTop ) + 'px';
            this.pinButton.src = this.options.pinImg;
        }
        this.wm.bury(this);
      }
      return false;
    },

    fillBody: function(id){
      var bodyElems = $$('#'+ id + ' ' + this.options.bodySelect);
      if ($defined(bodyElems) && bodyElems.length > 0) {
        this.winBody.empty();
        this.fireEvent('onDomExtend', [id, bodyElems]);
        this.winBody.adopt(bodyElems);
        var history_num_back = 0;
        var history_back = this.winBody.getElementById('history_' + id);
        if (history_back) {
          history_num_back = Number(history_back.value);
        }
        var confirm_del = this.winBody.getElementById('confirm_' + id);
        if (confirm_del && id.startsWith("del_") ) {
          confirm_del.onclick = null;
          confirm_del.addEvent('click', async function(event) {
              var err = await execute('delete_recording.html?param=' + id.substring(4) );
              if (!err.success) alert (err.error);
              if (history_num_back > 0) { history.go(-history_num_back); }
              else { location.reload(); }
              var event = new Event(event);
              event.stop();
              return this.hide();
            }.bind(this));
        }
        var close_button = this.winBody.getElementById('close_' + id);
        if (close_button) {
          close_button.onclick = null;
          close_button.addEvent('click', function(event){
              var event = new Event(event);
              event.stop();
              return this.hide();
            }.bind(this));
        }
        var firstScript = bodyElems.getElement('script.injectIcons');
        if (firstScript && firstScript.length && firstScript[0]) {
          var js_m = new Element('div').adopt(firstScript).firstChild.textContent;
          eval(js_m);
        }
        return true;
      }
      return false;
    },

    fillTitle: function(id){
      var titleElems = $$('#' + id + ' ' + this.options.titleSelect);
      if ($defined(titleElems) && titleElems.length > 0) {
        this.titleBox.empty().adopt(titleElems);
        return true;
      }
      return false;
    },

    position: function(event){
      var prop = {'x': 'left', 'y': 'top'};
      var pos = event.page['y'] + this.options.offsets['y'];
      content = document.getElementById('content');
      contentRect = content.getBoundingClientRect();
      if (pos < contentRect.y) pos = contentRect.y;
      this.winFrame.setStyle(prop['y'], pos);
      pos = event.page['x'] + this.options.offsets['x'];
      var width = this.winFrame.getBoundingClientRect().width;
      if (pos > window.innerWidth - width) pos = window.innerWidth - width;
      if (pos < 1) pos = 1;
      this.winFrame.setStyle(prop['x'], pos);
    }
  });

InfoWin.implement(new Events, new Options);

/*
Class: InfoWin.Manager
  Provide an container and events for the created info win
  instances.  Closed info-wins are preserved in a hidden DOM element
  and used again if a window with a closed id is opened again.
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
            this[wins].inject(document.getElementById('content') || document.body);
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
function is_digit(c){
  if (c >= '0' && c <= '9') {
    return true;
  } else {
    return false;
  }
}
function decrease_history_num_back(url) {
  var ind_history = url.indexOf("history_num_back=");
  if (ind_history == -1) return url;
  ind_history += 17;
  for (var ind_history_e = ind_history; ind_history_e < url.length && is_digit(url.substring(ind_history_e, ind_history_e+1)); ++ind_history_e);
  if (ind_history_e <= ind_history) return url;
  var history_num_back = Number(url.substring(ind_history, ind_history_e))-1;
  if (history_num_back < 0) return url;
  return url.substring(0, ind_history) + history_num_back + url.substring(ind_history_e);
}

InfoWin.Ajax = InfoWin.extend({
    options: {
      loadingMsg: 'loading',
      errorMsg: 'an error occurred!',
      onError: Class.empty
    },

    initialize: function(id, url_in, options){
      var url = decrease_history_num_back(url_in);
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
    // created a DOM subtree for an infowin.
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

  Creates a notification popup that disappears automatically.
  Useful for a confirmation message after a AJAX action request.
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
      /* body of tip: some helper divs and content */
      this.winBody = new Element('div', {
          'class': this.options.className + this.options.classSuffix + '-body'
        }).inject(this.winFrame);
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
