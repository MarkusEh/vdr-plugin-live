/*
 * Extension of mootools Tips class for rounded corner
 * tooltips of variable size up to some maximum.
 */

var HintTips = Tips.extend({
	  initialize: function(elements, options){
			this.parent(elements, options);
			this.toolTip.empty();
			/* top border of tip */
			var hd = new Element('div', {'class': this.options.className + '-tip-top'}).inject(this.toolTip);
			hd = new Element('div', {'class': this.options.className + '-tip-c'}).inject(hd);

			/* body of tip: some helper divs and content */
			this.wrapper = new Element('div', {'class': this.options.className + '-tip-bdy'}).inject(this.toolTip);
			this.wrapper = new Element('div', {'class': this.options.className + '-tip-c'}).inject(this.wrapper);
			this.wrapper = new Element('div', {'class': this.options.className + '-tip-s'}).inject(this.wrapper);

			/* bottom border of tip */
			var bt = new Element('div', {'class': this.options.className + '-tip-bot'}).inject(this.toolTip);
			bt = new Element('div', {'class': this.options.className + '-tip-c'}).inject(bt);
		}
	});

window.addEvent('domready', function(){
		var tips = new HintTips($$('*[title]'), {
			  maxTitleChars: 100,
			  className: 'hint'
			});
	});

window.addEvent('mousedown', function(){
		$$('.hint-tip').setStyle('visibility', 'hidden');
	});