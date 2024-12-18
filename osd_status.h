#ifndef VDR_LIVE_OSD_STATUS_H
#define VDR_LIVE_OSD_STATUS_H

// STL headers need to be before VDR tools.h (included by <vdr/status.h>)
#include <string>
#include <cxxtools/log.h>

#include "stringhelpers.h"
#include "tools.h"
#include <vdr/status.h>


namespace vdrlive {

class cLiveOsdItem: public cListObject {
  private:
    std::string text;
    bool selected;
    bool selectable;
  public:
    cSv Text() const { return text; }
    int  isSelected() const {return selected; }
    bool isSelectable() const {return selectable; }
    void Select(const bool doSelect) { selected= doSelect; };
    void Update(const char* Text);
    explicit cLiveOsdItem(const char* Text, bool Selectable):text(cSv(Text)),selected(false),selectable(Selectable) {}
    ~cLiveOsdItem() { }
};

class OsdStatusMonitor: public cStatus
{
  friend OsdStatusMonitor& LiveOsdStatusMonitor();
  OsdStatusMonitor();
  OsdStatusMonitor( OsdStatusMonitor const& );

  std::string title;
  std::string message;
  std::string red;
  std::string green;
  std::string yellow;
  std::string blue;
  std::string text;
  std::string channel_text;
  time_t present_time;
  std::string present_title;
  std::string present_subtitle;
  time_t following_time;
  std::string following_title;
  std::string following_subtitle;

  int selected;
  cList<cLiveOsdItem> items;
  clock_t lastUpdate;

public:
template <size_t N> cToSvConcat<N>& appendTitleHtml(cToSvConcat<N>& target) {
    if (title.empty() ) return target;
    target << "<div class=\"osdTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, title);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendMessageHtml(cToSvConcat<N>& target) {
    if (message.empty() ) return target;
    target << "<div class=\"osdMessage\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, message);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendRedHtml(cToSvConcat<N>& target) {
    if (red.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonRed\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, red);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendGreenHtml(cToSvConcat<N>& target) {
    if (green.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonGreen\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, green);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendYellowHtml(cToSvConcat<N>& target) {
    if (yellow.empty() ) {
      target << "<div class=\"osdButtonInvisible\"></div>";
    } else {
      target << "<div class=\"osdButtonYellow\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, yellow);
      target << "</div>";
    }
    return target;
  }
template <size_t N> cToSvConcat<N>& appendBlueHtml(cToSvConcat<N>& target) {
    if (blue.empty() ) return target;
    target << "<div class=\"osdButtonBlue\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, blue);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendButtonsHtml(cToSvConcat<N>& target) {
    if (red.empty() && green.empty() && yellow.empty() && blue.empty() ) return target;
    target << "<div class=\"osdButtons\">";
    appendRedHtml(target);
    appendGreenHtml(target);
    appendYellowHtml(target);
    appendBlueHtml(target);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendTextHtml(cToSvConcat<N>& target) {
    if (text.empty() ) return target;
    target << "<div class=\"osdText\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, text);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendChannelTextHtml(cToSvConcat<N>& target) {
    if (channel_text.empty() ) return target;
    target << "<div class=\"osdChannelText\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, channel_text);
    target << "</div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendProgrammeHtml(cToSvConcat<N>& target) {
    if (!present_time || present_title.empty() ) return target;
    target << "<div class=\"osdProgramme\"><table><tr><td>";
    target.appendDateTime(tr("%I:%M %p"), present_time);
    target << "</td><td>";
    target << "<div class=\"osdProgrammeTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, present_title);
    target << "</div><div class=\"osdProgrammeSubTitle\">";
    AppendHtmlEscapedAndCorrectNonUTF8(target, present_subtitle);
    target << "</div></td></tr>";
    if (following_time && !following_title.empty() ) {
      target << "<tr><td>";
      target.appendDateTime(tr("%I:%M %p"), following_time);
      target << "</td><td>";
      target << "<div class=\"osdProgrammeTitle\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, following_title);
      target << "</div><div class=\"osdProgrammeSubTitle\">";
      AppendHtmlEscapedAndCorrectNonUTF8(target, following_subtitle);
      target << "</div></td></tr>";
    }
    target << "</table></div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendItemsHtml(cToSvConcat<N>& target) {
    bool first = true;
    for (cLiveOsdItem *item = items.First(); item; item = items.Next(item)) {
      if (first) {
        first = false;
        target += "<div class=\"osdItems\"><table>";
      }
      target += "<tr class=\"osdItem";
      if (item->isSelected() ) target += " selected";
      if (!item->isSelectable() ) target += " notSelectable";
      target += "\">";
      for (cSv tc: cSplit(item->Text(), '\t')) {
        target += "<td>";
        AppendHtmlEscapedAndCorrectNonUTF8(target, tc);
        target += "</td>";
      }
      target += "</tr>";
    }
    if (!first) target += "</table></div>";
    return target;
  }
template <size_t N> cToSvConcat<N>& appendHtml(cToSvConcat<N>& target) {
    target << "<div class=\"osd\" data-time=\"" << lastUpdate << "\">";
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
