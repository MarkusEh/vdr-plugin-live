/*
 * This is part of the live VDR plugin. See COPYING for license information.
 *
 * HintTips class.
 *
 * Extension of mootools Tips class for rounded corner tooltips of
 * variable size up to some maximum.
 */

var HintTips = Tips.extend({
    initialize: function(elements, options){
      this.parent(elements, options);
      this.toolTip.empty();
      this.wrapper = new Element('div', {'class': this.options.className + '-tip-body'}).inject(this.toolTip);
    }
  });
