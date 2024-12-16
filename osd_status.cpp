#include "osd_status.h"

#include <sstream>

namespace vdrlive {

OsdStatusMonitor::OsdStatusMonitor():title(),message(),red(),green(),yellow(),blue(),text(),selected(-1),lastUpdate(0){
}
OsdStatusMonitor::~OsdStatusMonitor() {
  OsdClear();
}

void OsdStatusMonitor::OsdClear() {
  title = message = text = "";
  red = green = yellow = blue = "";
  items.Clear();
  selected = -1;
  lastUpdate= clock();
}

void OsdStatusMonitor::OsdTitle(const char *Title) {
  if (Title) {
    if (title != Title) title = Title;
  } else {
    if (!title.empty() ) title.clear();
  }
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

/* documentation in vdr source:
  virtual void OsdItem(const char *Text, int Index) {}
    // The OSD displays the given single line Text as menu item at Index.
*/
void OsdStatusMonitor::OsdItem(const char *Text, int Index) {
  items.Add(new cLiveOsdItem(Text));
  lastUpdate= clock();
}

void OsdStatusMonitor::OsdCurrentItem(const char *Text) {
  int i = -1;
  int best = -1;
  int dist = items.Count();
  cLiveOsdItem *currentItem = NULL;
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

/* documentation in vdr source:
  virtual void OsdTextItem(const char *Text, bool Scroll) {}
     // The OSD displays the given multi line text. If Text points to an
     // actual string, that text shall be displayed and Scroll has no
     // meaning. If Text is NULL, Scroll defines whether the previously
     // received text shall be scrolled up (true) or down (false) and
     // the text shall be redisplayed with the new offset.
*/

void OsdStatusMonitor::OsdTextItem(const char *Text, bool Scroll) {
  if (Text) {
    if (text != Text) text = Text;
  }
// Ignore if called with Text == nullptr
//   accoeding to doc, the previously received text shall be scrolled up (true) or down (false)
//   we use scroll bar for that ...
  lastUpdate= clock();
}

OsdStatusMonitor& LiveOsdStatusMonitor()
{
  static OsdStatusMonitor instance;
  return instance;
}

} // namespace vdrlive
