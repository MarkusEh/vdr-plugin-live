#ifndef VDR_LIVE_OSD_STATUS_H
#define VDR_LIVE_OSD_STATUS_H

// STL headers need to be before VDR tools.h (included by <vdr/status.h>)
#include <string>

#include <vdr/status.h>


namespace vdrlive {

class cLiveOsdItem: public cListObject {
	private:
		std::string text;
		bool selected;
	public:
		std::string Text() const { return text; }
		int	isSelected() const {return selected;}
		void Select(const bool doSelect) { selected= doSelect; };
		void Update(const char* Text) { text = Text ? Text : ""; };
		explicit cLiveOsdItem(const char* Text):text(),selected(false) { text = Text ? Text : ""; };
		~cLiveOsdItem() { }
};

class OsdStatusMonitor: public cStatus
{
	friend OsdStatusMonitor& LiveOsdStatusMonitor();
public:
		enum { MaxTabs = 6 };
private:
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
	cList<cLiveOsdItem>	items;
	unsigned short tabs[MaxTabs];
	clock_t lastUpdate;

protected:
//	static void append(char *&tail, char type, const char *src, int max);
public:	

	std::string const GetTitle() const {return title;}
	std::string const GetMessage() const {return message;}
	std::string const GetRed() const {return red;}
	std::string const GetGreen() const {return green;}
	std::string const GetYellow() const {return yellow;}
	std::string const GetBlue() const {return blue;}
	std::string const GetText() const {return text;}

	virtual std::string const GetHtml();
	virtual std::string const GetTitleHtml();
	virtual std::string const GetMessageHtml();
	virtual std::string const GetRedHtml();
	virtual std::string const GetGreenHtml();
	virtual std::string const GetYellowHtml();
	virtual std::string const GetBlueHtml();
	virtual std::string const GetTextHtml();
	virtual std::string const GetButtonsHtml();
	virtual std::string const GetItemsHtml();

	virtual void OsdClear();
	virtual void OsdTitle(const char *Title);
	virtual void OsdStatusMessage(const char *Message);
	virtual void OsdHelpKeys(const char *Red, const char *Green, const char *Yellow, const char *Blue);
	virtual void OsdTextItem(const char *Text, bool Scroll);
	virtual void OsdItem(const char *Text, int Index);
	virtual void OsdCurrentItem(const char *Text);

	virtual ~OsdStatusMonitor();

	std::string const EncodeHtml(const std::string& html);
};

OsdStatusMonitor& LiveOsdStatusMonitor();

} // namespace vdrlive

#endif // VDR_LIVE_STATUS_H
