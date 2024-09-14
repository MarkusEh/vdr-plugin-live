/*
 * This is part of the live VDR plugin. See COPYING for license information.
 *
 * PageEnhance class.
 *
 * This class applies several functions to the page based on
 * selectors. This class is project dependent and not a general
 * purpose class.
 */

var PageEnhance = new Class({
    options: {
      epgLinkSelector: 'a[href^="epginfo.html?epgid"]',
      actionLinkSelector: 'a[href^="vdr_request/"]',
      editTimerSelector: 'a[href^="edit_timer.html?timerid"]',
      hintTipSelector: '*[title]',
      hintClassName: 'hint',
      infoWinOptions: {
        bodyselect: 'div.epg_content',
        loadingMsg: 'loading',
        errorMsg: 'an error occurred!'
      },
      notifyIdPrefix: 'notify',
      notifyStrings: {
        successMsg: '<img src="active.png" alt=""> Success!',
        errorMsg: '<img src="del.png" alt=""> failed!'
      }
    },

    initialize: function(options){
      this.setOptions(options);
      this.$notifyCount = 0;
      window.addEvent('domready', this.domReadySetup.bind(this));
      window.addEvent('mousedown', this.mouseDownSetup.bind(this));
    },

    // actions applied on domready to the page.
    domReadySetup: function(){
      $$(this.options.epgLinkSelector).each(this.epgPopup.bind(this));
      this.addHintTips($$(this.options.hintTipSelector));
      $$(this.options.actionLinkSelector).each(this.vdrRequest.bind(this));
      // the following line activates timer editing in popup window.
      // but it does not yet work like expected. So we leave it deactivated currently.
      // $$(this.options.editTimerSelector).each(this.editTimer.bind(this));
    },

    // actions applied on mouse down.
    mouseDownSetup: function(){
      $$('.' + this.options.hintClassName + '-tip').setStyle('visibility', 'hidden');
    },

    // registered as 'onDomExtend' event for InfoWin. Takes care to
    // enhance the new DOM elements, too.
    domExtend: function(id, elems){
      var sel = '#' + id + ' ' + this.options.hintTipSelector;
      elems = $$(sel);
      this.addHintTips(elems);
      $$('#' + id + ' ' + this.options.actionLinkSelector).each(this.vdrRequest.bind(this));
    },

    // EPG popup function. Apply to all elements that should
    // pop up an EPG InfowWin window.
    epgPopup: function(el){
      var href = el.href;
      var epgid = $pick(href, "");
      if (epgid != "") {
        var extractId = /epgid=(\w+)/;
        var found = extractId.exec(epgid);
        if ($defined(found) && found.length > 1) {
          epgid = found[1];
          el.addEvent('click', function(event){
              if (window.matchMedia("(max-width: 600px)").matches) {
                location.replace(href);
                return true;
              }
              var event = new Event(event);
              new InfoWin.Ajax(epgid, href, $merge(this.options.infoWinOptions, {
                    onDomExtend: this.domExtend.bind(this)
                      })).show(event);
              event.stop();
              return false;
            }.bind(this));
        }
      }
    },

    // Edit Timer Popup function. Apply to all elements that should
    // pop up a timer edit windows based on InfoWin window.
    editTimer: function(el){
      var href = el.href;
      var timerid = $pick(href, "");
      if (timerid != "") {
        var extractId = /timerid=(.+)/;
        var found = extractId.exec(timerid);
        if ($defined(found) && found.length > 1) {
          timerid = found[1];
          el.addEvent('click', function(event){
              var event = new Event(event);
              new InfoWin.Ajax(timerid, href, $merge(this.options.infoWinOptions, {
                    bodyselect: '',
                    modal: true,
                    onDomExtend: this.domExtend.bind(this)
                      })).show(event);
              event.stop();
              return false;
            }.bind(this));
        }
      }
    },

    // function that requests an action from the server VDR.
    vdrRequest: function(el){
      el.addEvent('click', function(event, element){
          var href = $pick(element.href, "");
          if (href != "") {
            this.$notifyCount++;
            var req = new Ajax(href, {
                method: 'post',
                onComplete: function(text, xmldoc) {
                  try {
                    var success = xmldoc.getElementsByTagName('response').item(0).firstChild.nodeValue;
                    new InfoWin.Notifier(this.options.notifyIdPrefix + this.$notifyCount, {
                        className: success == '1' ? 'ok' : 'err',
                        message: success == '1' ? this.options.notifyStrings.successMsg : this.options.notifyStrings.errorMsg
                      }).show(event);
                  } catch(e) {
                  }
                }.bind(this)
              });
            req.request('async=1');
            event.stop();
            return false;
          }
          return true;
        }.bindWithEvent(this, el));
    },

    // change normal 'title'-Attributes into enhanced hinttips
    // used by domExtend and domReadySetup functions.
    addHintTips: function(elems) {
      if (window.matchMedia("(hover: none)").matches) {
      elems_use = elems.filter(
        function(item, index){ return !item.hasClass('apopup'); }
        );
          } else {
            elems_use = elems;
           }
      if (!$defined(this.tips)) {
        this.tips = new HintTips(elems_use, {
            maxTitleChars: 100,
            className: this.options.hintClassName
          });
      }
      else {
        $$(elems_use).each(this.tips.build, this.tips);
      }
    }
  });

PageEnhance.implement(new Events, new Options);
