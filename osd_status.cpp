#include "osd_status.h"

#include <sstream>

namespace vdrlive {

OsdStatusMonitor::OsdStatusMonitor():present_time(0),following_time(0),selected(-1),lastUpdate(0) {}
OsdStatusMonitor::~OsdStatusMonitor() {
  OsdClear();
}

void OsdStatusMonitor::OsdClear() {
  title = message = text = channel_text = present_title = present_subtitle = following_title = following_subtitle = "";
  red = green = yellow = blue = "";
  items.Clear();
  present_time = 0;
  following_time = 0;
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

void cLiveOsdItem::Update(const char* Text) {
  if (Text) {
    if (text != Text) text = Text;
  } else {
    if (!text.empty() ) text.clear();
  }
}

/* documentation in vdr source:
  virtual void OsdItem(const char *Text, int Index) {}
    // The OSD displays the given single line Text as menu item at Index.
*/
#if defined(OSDITEM) && OSDITEM == 2
void OsdStatusMonitor::OsdItem2(const char *Text, int Index, bool Selectable) {
  items.Add(new cLiveOsdItem(Text,Selectable));
#else
void OsdStatusMonitor::OsdItem(const char *Text, int Index) {
  items.Add(new cLiveOsdItem(Text,true));
#endif
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
void OsdStatusMonitor::OsdChannel(const char *Text) {
  if (Text) {
    if (channel_text != Text) channel_text = Text;
  } else {
    if (!channel_text.empty() ) channel_text.clear();
  }
  lastUpdate= clock();
}

void OsdStatusMonitor::OsdProgramme(time_t PresentTime, const char *PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle) {
  present_time = PresentTime;
  present_title = cSv(PresentTitle);
  present_subtitle = cSv(PresentSubtitle);
  following_time = FollowingTime;
  following_title = cSv(FollowingTitle);
  following_subtitle = cSv(FollowingSubtitle);

  lastUpdate= clock();
}

OsdStatusMonitor& LiveOsdStatusMonitor()
{
  static OsdStatusMonitor instance;
  return instance;
}

} // namespace vdrlive
