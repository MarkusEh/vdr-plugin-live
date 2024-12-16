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
  public:
    std::string Text() const { return text; }
    int  isSelected() const {return selected;}
    void Select(const bool doSelect) { selected= doSelect; };
    void Update(const char* Text) { text = Text ? Text : ""; };
    explicit cLiveOsdItem(const char* Text):text(),selected(false) { text = Text ? Text : ""; };
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
  int selected;
  cList<cLiveOsdItem> items;
  clock_t lastUpdate;

public:

/*
  std::string const GetMessage() const {return message;}
  std::string const GetRed() const {return red;}
  std::string const GetGreen() const {return green;}
  std::string const GetYellow() const {return yellow;}
  std::string const GetBlue() const {return blue;}
  std::string const GetText() const {return text;}
*/

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
template <size_t N> cToSvConcat<N>& appendItemsHtml(cToSvConcat<N>& target) {
    bool first = true;
    std::string text;
    for (cLiveOsdItem *item = items.First(); item; item = items.Next(item)) {
      text = item->Text();
      bool selected = item->isSelected();
      if (first) {
        first = false;
        target += "<div class=\"osdItems\"><table>";
      }
      target += "<tr class=\"osdItem";
      if (selected) target += " selected";
      target += "\">";
      for (cSv tc: cSplit(text, '\t')) {
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
    appendMessageHtml(target);
    appendButtonsHtml(target);
    target << "</div>";
    return target;
  }

  virtual void OsdClear();
  virtual void OsdTitle(const char *Title);
  virtual void OsdStatusMessage(const char *Message);
  virtual void OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue);
  virtual void OsdTextItem(const char *Text, bool Scroll);
  virtual void OsdItem(const char *Text, int Index);
  virtual void OsdCurrentItem(const char *Text);

  virtual ~OsdStatusMonitor();
};

OsdStatusMonitor& LiveOsdStatusMonitor();

} // namespace vdrlive

#endif // VDR_LIVE_STATUS_H
