#include "osd_status.h"
#include <vdr/thread.h>

namespace vdrlive {

cOsdStatusMonitorLock::cOsdStatusMonitorLock(bool Write) {
  LiveOsdStatusMonitor().m_stateLock.Lock(m_stateKey, Write);
}

OsdStatusMonitor::OsdStatusMonitor():m_present_time(0),m_following_time(0),m_selected(-1),m_lastUpdate(0) {}
OsdStatusMonitor::~OsdStatusMonitor() {
  OsdClear();
}

void OsdStatusMonitor::OsdClear() {
  cOsdStatusMonitorLock lw(true);
  m_title = m_message = m_text = m_channel_text = m_present_title = m_present_subtitle = m_following_title = m_following_subtitle = "";
  m_red = m_green = m_yellow = m_blue = "";
  m_items.clear();
  m_present_time = 0;
  m_following_time = 0;
  m_selected = -1;
  m_lastUpdate= clock();
}

void OsdStatusMonitor::OsdTitle(const char *Title) {
  cOsdStatusMonitorLock lw(true);
  if (Title) {
    if (m_title != Title) m_title = Title;
  } else {
    if (!m_title.empty() ) m_title.clear();
  }
  m_lastUpdate= clock();
}

#if VDRVERSNUM >= 20705
void OsdStatusMonitor::OsdStatusMessage(eMessageType Type, const char *Message) {
  cOsdStatusMonitorLock lw(true);
  m_message_type = Type;
#else
void OsdStatusMonitor::OsdStatusMessage(const char *Message) {
  cOsdStatusMonitorLock lw(true);
#endif
  m_message = Message ? Message : "";
  m_lastUpdate= clock();
}

void OsdStatusMonitor::OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
  cOsdStatusMonitorLock lw(true);
  m_red = Red ? Red :"";
  m_green = Green ? Green : "";
  m_yellow = Yellow ? Yellow : "";
  m_blue = Blue ? Blue : "";
  m_lastUpdate= clock();
}

void cLiveOsdItem::Update(const char* Text) {
// do not lock here: this is called by other methods having already the lock
  if (Text) {
    if (m_text != Text) m_text = Text;
  } else {
    if (!m_text.empty() ) m_text.clear();
  }
}

/* documentation in VDR source:
  virtual void OsdItem(const char *Text, int Index) {}
    // The OSD displays the given single line Text as menu item at Index.
*/
#if VDRVERSNUM >= 20705
void OsdStatusMonitor::OsdItem(const char *Text, int Index, bool Selectable) {
  cOsdStatusMonitorLock lw(true);
  m_items.emplace_back(Text,Selectable);
#else
void OsdStatusMonitor::OsdItem(const char *Text, int Index) {
  cOsdStatusMonitorLock lw(true);
  m_items.emplace_back(Text,true);
#endif
  m_lastUpdate= clock();
}

#if VDRVERSNUM >= 20705
void OsdStatusMonitor::OsdCurrentItem(const char *Text, int Index) {
  cOsdStatusMonitorLock lw(true);
  if (Index >= 0) m_selected = Index;
  if (Text) {
    if (m_selected < 0)
      esyslog("live: ERROR, OsdStatusMonitor::OsdItemChanged, m_selected < 0, Text = %s", Text);
    else
      m_items[m_selected].Update(Text);
  }
  m_lastUpdate= clock();
}
#else
bool OsdStatusMonitor::Select_if_matches(std::vector<cLiveOsdItem>::size_type line, const char *Text) {
// if matches, select line and return true
// otherwise, return false
  if (!m_items[line].isSelectable() || m_items[line].Text().compare(Text) != 0) return false;
  m_selected = line;
  m_lastUpdate= clock();
  return true;
}
/* documentation in VDR source:
     The OSD displays the given single line Text as the current menu item.

  And now the details from VDR source:
     This is called in 2 cases:
       a) another item in the current list is selected
          see osdbase.c: cOsdMenu::Display(), cOsdMenu::DisplayCurrent(bool Current), cOsdMenu::DisplayItem(cOsdItem *Item)
       b) a text of a line item is changed (more precise: value of setting is changed)
          see menuitems.c -> void cMenuEditItem::SetValue(const char *Value)

  Implementation:
    first check whether the current item list contains a line with the given text
      If this is the case, we assume a) and select this line item
      If this is not the case, we assume b) and we change the text of the currently selected line

   Note: text of a line item is changed also during creation of a new item. So,
     while creating a new menu, there will be several calls to OsdCurrentItem
     changing text of items currently not displayed ...

     This is not intended, so, before changing the text, we check whether the
     attribute name (this is the text before tab) matches

*/

void OsdStatusMonitor::OsdCurrentItem(const char *Text) {
  if (!Text) return;
  cOsdStatusMonitorLock lw(true);
  if (m_selected < 0) {
// previous position unknown - take first match
    for (std::vector<cLiveOsdItem>::size_type item_n = 0; item_n < m_items.size(); ++item_n)
      if (Select_if_matches(item_n, Text) ) break;
    return;
  }
  if (m_items[m_selected].Text().compare(Text) == 0) return; // currently selected item matches Text

  int item_prev = m_selected-1;
  std::vector<cLiveOsdItem>::size_type item_next = m_selected+1;
  for (;item_prev >= 0 || item_next < m_items.size(); --item_prev, ++item_next) {
    if (item_next < m_items.size() && Select_if_matches(item_next, Text)) return;
    if (item_prev >= 0             && Select_if_matches(item_prev, Text)) return;
  }
// no match: -> case b), the same item is still selected but its text changed
  if (*cSplit(Text, '\t').begin() == *cSplit(m_items[m_selected].Text(), '\t').begin() ) {
// update value of setting
    m_items[m_selected].Update(Text);
    m_lastUpdate= clock();
  } else {
//  esyslog("live, OsdStatusMonitor::OsdCurrentItem, Text = \"%s\", current text: \"%.*s\"", Text, (int)m_items[m_selected].Text().length(), m_items[m_selected].Text().data() );
  }
}
#endif

/* documentation in VDR source:
  virtual void OsdTextItem(const char *Text, bool Scroll) {}
     // The OSD displays the given multi line text. If Text points to an
     // actual string, that text shall be displayed and Scroll has no
     // meaning. If Text is NULL, Scroll defines whether the previously
     // received text shall be scrolled up (true) or down (false) and
     // the text shall be redisplayed with the new offset.
*/

void OsdStatusMonitor::OsdTextItem(const char *Text, bool Scroll) {
  cOsdStatusMonitorLock lw(true);
  if (Text) {
    if (m_text != Text) m_text = Text;
    m_lastUpdate= clock();
  }
// Ignore if called with Text == nullptr
//   according to doc, the previously received text shall be scrolled up (true) or down (false)
//   we use scroll bar for that ...
}
void OsdStatusMonitor::OsdChannel(const char *Text) {
  cOsdStatusMonitorLock lw(true);
  if (Text) {
    if (m_channel_text != Text) m_channel_text = Text;
  } else {
    if (!m_channel_text.empty() ) m_channel_text.clear();
  }
  m_lastUpdate= clock();
}

void OsdStatusMonitor::OsdProgramme(time_t PresentTime, const char *PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle) {
  cOsdStatusMonitorLock lw(true);
  m_present_time = PresentTime;
  m_present_title = cSv(PresentTitle);
  m_present_subtitle = cSv(PresentSubtitle);
  m_following_time = FollowingTime;
  m_following_title = cSv(FollowingTitle);
  m_following_subtitle = cSv(FollowingSubtitle);

  m_lastUpdate= clock();
}

OsdStatusMonitor& LiveOsdStatusMonitor()
{
  static OsdStatusMonitor instance;
  return instance;
}

} // namespace vdrlive
