#ifndef VDR_LIVE_OSD_STATUS_H
#define VDR_LIVE_OSD_STATUS_H

// STL headers need to be before VDR tools.h (included by <vdr/status.h>)
#include <string>
#include <cxxtools/log.h>

#include "stringhelpers.h"
#include "tools.h"
#include <vdr/status.h>


namespace vdrlive {

class cOsdStatusMonitorLock {
  private:
    cStateKey m_stateKey;
  public:
    cOsdStatusMonitorLock(bool Write = false);
    ~cOsdStatusMonitorLock() { m_stateKey.Remove(); }
};

class cLiveOsdItem {
  private:
    std::string m_text;
    bool m_selectable;
  public:
    cSv Text() const { return m_text; }
    bool isSelectable() const {return m_selectable; }
    void Update(const char* Text);
    explicit cLiveOsdItem(cSv Text, bool Selectable):m_text(Text),m_selectable(Selectable) {}
    ~cLiveOsdItem() { }
};

class OsdStatusMonitor: public cStatus
{
  friend OsdStatusMonitor& LiveOsdStatusMonitor();
  friend cOsdStatusMonitorLock;
  OsdStatusMonitor();
  OsdStatusMonitor( OsdStatusMonitor const& );

  std::string m_title;
  std::string m_message;
  std::string m_red;
  std::string m_green;
  std::string m_yellow;
  std::string m_blue;
  std::string m_text;
  std::string m_channel_text;
  time_t m_present_time;
  std::string m_present_title;
  std::string m_present_subtitle;
  time_t m_following_time;
  std::string m_following_title;
  std::string m_following_subtitle;

  int m_selected;
  std::vector<cLiveOsdItem> m_items;
  clock_t m_lastUpdate;
  mutable cStateLock m_stateLock;

public:
  clock_t getLastUpdate() const { cOsdStatusMonitorLock lr; clock_t r = m_lastUpdate; return r; }
template <size_t N> cToSvConcat<N>& appendTitleHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_title.empty() ) return target;
    target << "<div class=\"osdTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_title);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendMessageHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_message.empty() ) return target;
    target << "<div class=\"osdMessage\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_message);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendRedHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_red.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonRed\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, m_red);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendGreenHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_green.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonGreen\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, m_green);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendYellowHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_yellow.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonYellow\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, m_yellow);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendBlueHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_blue.empty() ) return target;
    target << "<div class=\"osdButtonBlue\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_blue);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendButtonsHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_red.empty() && m_green.empty() && m_yellow.empty() && m_blue.empty() ) return target;
    target << "<div class=\"osdButtons\">";
    appendRedHtml(target);
    appendGreenHtml(target);
    appendYellowHtml(target);
    appendBlueHtml(target);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendTextHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_text.empty() ) return target;
    target << "<div class=\"osdText\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_text);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendChannelTextHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_channel_text.empty() ) return target;
    target << "<div class=\"osdChannelText\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_channel_text);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendProgrammeHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (!m_present_time || m_present_title.empty() ) return target;
    target << "<div class=\"osdProgramme\"><table><tr><td>";
    target.appendDateTime(tr("%I:%M %p"), m_present_time);
    target << "</td><td>";
    target << "<div class=\"osdProgrammeTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_present_title);
    target << "</div><div class=\"osdProgrammeSubTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, m_present_subtitle);
    target << "</div></td></tr>";
    if (m_following_time && !m_following_title.empty() ) {
      target << "<tr><td>";
      target.appendDateTime(tr("%I:%M %p"), m_following_time);
      target << "</td><td>";
      target << "<div class=\"osdProgrammeTitle\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, m_following_title);
      target << "</div><div class=\"osdProgrammeSubTitle\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, m_following_subtitle);
      target << "</div></td></tr>";
    }
    target << "</table></div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendItemsHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    if (m_items.empty()) return target;
    target += "<div class=\"osdItems\"><table>";
    for (std::vector<cLiveOsdItem>::size_type item_n = 0; item_n < m_items.size(); ++item_n) {
      target += "<tr class=\"osdItem";
      if ((int)item_n == m_selected) target += " selected";
      if (!m_items[item_n].isSelectable() ) target += " notSelectable";
      target += "\">";
      bool hasColumns = m_items[item_n].Text().find('\t') != std::string::npos;
      for (cSv tc: cSplit(m_items[item_n].Text(), '\t')) {
        target += hasColumns ? "<td>" : "<td colspan=\"100%\">";
        AppendHtmlEscapedAndCorrectNonUTF8(target, tc);
        target += "</td>";
      }
      target += "</tr>";
    }
    target += "</table></div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendHtml(cToSvConcat<N>& target) {
    cOsdStatusMonitorLock lr;
    target << "<div class=\"osd\" data-time=\"" << m_lastUpdate << "\">";
    appendTitleHtml(target);
    appendItemsHtml(target);
    appendTextHtml(target);
    appendChannelTextHtml(target);
    appendProgrammeHtml(target);
    appendMessageHtml(target);
    appendButtonsHtml(target);
    target << "</div>";
    return target;
  }

  virtual void OsdClear(void);
               // The OSD has been cleared.
  virtual void OsdTitle(const char *Title);
               // Title has been displayed in the title line of the menu.
  virtual void OsdStatusMessage(const char *Message);
               // Message has been displayed in the status line of the menu.
               // If Message is NULL, the status line has been cleared.
  virtual void OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue);
               // The help keys have been set to the given values (may be NULL).
#if defined(OSDITEM) && OSDITEM == 2
  virtual void OsdItem2(const char *Text, int Index, bool Selectable);
#else
  virtual void OsdItem(const char *Text, int Index);
               // The OSD displays the given single line Text as menu item at Index.
#endif
  bool Select_if_matches(std::vector<cLiveOsdItem>::size_type line, const char *Text);
  virtual void OsdCurrentItem(const char *Text);
               // The OSD displays the given single line Text as the current menu item.
  virtual void OsdTextItem(const char *Text, bool Scroll);
               // The OSD displays the given multi line text. If Text points to an
               // actual string, that text shall be displayed and Scroll has no
               // meaning. If Text is NULL, Scroll defines whether the previously
               // received text shall be scrolled up (true) or down (false) and
               // the text shall be redisplayed with the new offset.
  virtual void OsdChannel(const char *Text);
               // The OSD displays the single line Text with the current channel information.
  virtual void OsdProgramme(time_t PresentTime, const char *PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle);
               // The OSD displays the given programme information.

  virtual ~OsdStatusMonitor();
};

OsdStatusMonitor& LiveOsdStatusMonitor();

} // namespace vdrlive

#endif // VDR_LIVE_STATUS_H
