
#include "osd_status.h"

#include <sstream>

namespace vdrlive {

OsdStatusMonitor::OsdStatusMonitor():title(),message(),red(),green(),yellow(),blue(),text(),selected(-1),lastUpdate(0){
	memset(&tabs, 0, sizeof(tabs));
}
OsdStatusMonitor::~OsdStatusMonitor() {
	OsdClear();
}	

void OsdStatusMonitor::OsdClear() {
	title = message = text = "";
	red = green = yellow = blue = "";
	items.Clear();
	selected = -1;
	memset(&tabs, 0, sizeof(tabs));
	lastUpdate= clock();
}

void OsdStatusMonitor::OsdTitle(const char *Title) {
	title = Title ? Title : "";
	lastUpdate= clock();
}

void OsdStatusMonitor::OsdStatusMessage(const char *Message) {
	message = Message ? Message : "";
	lastUpdate= clock();
}

void OsdStatusMonitor::OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
	red = Red ? Red :"";
	green = Green ? Green : "";
	yellow = Yellow ? Yellow : "";
	blue = Blue ? Blue : "";
	lastUpdate= clock();
}

void OsdStatusMonitor::OsdItem(const char *Text, int Index) {
	const char* tab;
	const char* colStart = Text;
	for (int col = 0; col < MaxTabs &&
			(tab = strchr(colStart, '\t')); col++) {
		int width = tab - colStart + 1;
		if (width > tabs[col])
			tabs[col] = width;
		colStart = colStart + width;
	}
	items.Add(new cLiveOsdItem(Text));
	lastUpdate= clock();
}

void OsdStatusMonitor::OsdCurrentItem(const char *Text) {
	int i = -1;
	int best = -1;
	int dist = items.Count();
	cLiveOsdItem * currentItem = NULL;
	cLiveOsdItem *bestItem = NULL;
	for (cLiveOsdItem *item = items.First(); item; item = items.Next(item)) {
		if (++i == selected)
			currentItem = item;
		if ( item->Text().compare(Text) == 0) {
			if (abs(i - selected) < dist) {
				// best match is the one closest to previous position
				best = i;
				bestItem= item;
				dist = abs(i - selected);
			}
			else if (selected < 0) {
				// previous position unknown - take first match
				best = i;
				bestItem= item;
				break;
			}
			else {
				// we already have a better match, so we're done
				break;
			}
		}
	}
	if (best >= 0) {
		// found matching item
		selected = best;
		bestItem->Select(true);
		if (currentItem && currentItem != bestItem){
			currentItem->Select(false);
			lastUpdate= clock();
		}
	}
	else if (currentItem) {
		// no match: the same item is still selected but its text changed
		currentItem->Update(Text);
		lastUpdate= clock();
	}
}

void OsdStatusMonitor::OsdTextItem(const char *Text, bool Scroll) {
	if (Text) {
		text = Text;
		//text= text.replace( text.begin(), text.end(), '\n', '|');
	}
	else {
		text = "";
	}
	lastUpdate= clock();
}
std::string const OsdStatusMonitor::GetTitleHtml() {return !title.empty() ? "<div class=\"osdTitle\">" + EncodeHtml(title) + "</div>" : "";}
std::string const OsdStatusMonitor::GetMessageHtml() {return !message.empty() ? "<div class=\"osdMessage\">" + EncodeHtml(message) + "</div>" : "";}
std::string const OsdStatusMonitor::GetRedHtml()  {return !red.empty() ? "<div class=\"osdButtonRed\">" + EncodeHtml(red) + "</div>" : "";}
std::string const OsdStatusMonitor::GetGreenHtml() {return !green.empty() ? "<div class=\"osdButtonGreen\">" + EncodeHtml(green) + "</div>" : "";}
std::string const OsdStatusMonitor::GetYellowHtml() {return !yellow.empty() ? "<div class=\"osdButtonYellow\">" + EncodeHtml(yellow) + "</div>" : "";}
std::string const OsdStatusMonitor::GetBlueHtml() {return !blue.empty() ? "<div class=\"osdButtonBlue\">" + EncodeHtml(blue) + "</div>" : "";}
std::string const OsdStatusMonitor::GetTextHtml() {return !text.empty() ? "<div class=\"osdText\">" +  EncodeHtml(text) + "</div>" : "";}
std::string const OsdStatusMonitor::GetButtonsHtml() {
	std::string buffer= GetRedHtml() + GetGreenHtml() + GetYellowHtml() + GetBlueHtml();
	return !buffer.empty() ? "<div class=\"osdButtons\">" + buffer + "</div>" : "";
}

std::string const OsdStatusMonitor::GetItemsHtml(void){
	std::string buffer= "";
	for (cLiveOsdItem *item = items.First(); item; item = items.Next(item)) {
		buffer += "<div class=\"osdItem";
		if (item->isSelected())
			buffer +=  " selected";
		buffer += "\">";
		buffer += EncodeHtml(item->Text());
		buffer += "</div>";
	}
	return !buffer.empty() ? "<div class=\"osdItems\">" + buffer + "</div>" : "";
}
std::string const OsdStatusMonitor::GetHtml(){
	std::stringstream ss;
	ss << lastUpdate;
	return "<div class=\"osd\" data-time=\"" + ss.str() + "\">" + GetTitleHtml() + GetItemsHtml() + GetTextHtml() + GetMessageHtml() + GetButtonsHtml() + "</div>";
}

std::string const OsdStatusMonitor::EncodeHtml(const std::string& html) {
	std::stringstream ss;
	std::string::const_iterator i;
	for (i = html.begin(); i != html.end(); ++i) {
		if (*i == '<')
			ss << "&lt;";
		else if (*i == '>')
			ss << "&gt;";
		else if (*i == '&')
			ss << "&amp;";
		else if (*i == '"')
			ss << "&quot;";
		else
			ss << static_cast<char>(*i); // Copy untranslated
	}
	return ss.str();
}



OsdStatusMonitor& LiveOsdStatusMonitor()
{
	static OsdStatusMonitor instance;
	return instance;
}

} // namespace vdrlive
