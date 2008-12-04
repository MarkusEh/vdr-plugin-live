#ifndef VDR_LIVE_EPGSEARCH_H
#define VDR_LIVE_EPGSEARCH_H

#include <vector>
#include <list>
#include <set>
#include <string>
#include <vdr/channels.h>
#include <vdr/epg.h>
#include "tools.h"

namespace vdrlive {

class SearchTimer;

bool operator<( SearchTimer const& left, SearchTimer const& right );

class SearchTimer
{
public:
	enum eUseChannel
	{
		NoChannel = 0,
		Interval = 1,
		Group = 2,
		FTAOnly = 3
	};
	
	SearchTimer();
	SearchTimer( std::string const& data );
	void Init();
	std::string ToText();
	friend bool operator<( SearchTimer const& left, SearchTimer const& right );

	int Id() const { return m_id; }
	void SetId(int id) { m_id = id; }
	std::string const& Search() const { return m_search; }
	void SetSearch(std::string const& search) { m_search = search; }
	int SearchMode() { return m_mode; }
	void SetSearchMode(int mode) { m_mode = mode; }
	int Tolerance() const { return m_fuzzytolerance; }
	void SetTolerance(int tolerance) { m_fuzzytolerance = tolerance; }
	bool MatchCase() const { return m_useCase; }
	void SetMatchCase(bool useCase) { m_useCase = useCase; }
	bool UseTime() const { return m_useTime; }
	void SetUseTime(bool useTime) { m_useTime = useTime; }
	bool UseTitle() const { return m_useTitle; }
	void SetUseTitle(bool useTitle) { m_useTitle = useTitle; }
	bool UseSubtitle() const { return m_useSubtitle; }
	void SetUseSubtitle(bool useSubtitle) { m_useSubtitle = useSubtitle; }
	bool UseDescription() const { return m_useDescription; }
	void SetUseDescription(bool useDescription) { m_useDescription = useDescription; }
	int StartTime() const { return m_startTime; }
	std::string StartTimeFormatted();
	void SetStartTime(int startTime) { m_startTime = startTime; }
	int StopTime() const { return m_stopTime; }
	std::string StopTimeFormatted();
	void SetStopTime(int stopTime) { m_stopTime = stopTime; }
	eUseChannel UseChannel() const { return static_cast< eUseChannel >( m_useChannel ); }
	void SetUseChannel(eUseChannel useChannel) { m_useChannel = useChannel; }
	tChannelID ChannelMin() const { return m_channelMin; }
	void SetChannelMin(tChannelID channelMin) { m_channelMin = channelMin; }
	tChannelID ChannelMax() const { return m_channelMax; }
	void SetChannelMax(tChannelID channelMax) { m_channelMax = channelMax; }
	std::string ChannelText() const { return m_channels; }
	void SetChannelText(std::string channels) { m_channels = channels; }
	int UseAsSearchTimer() const { return m_useAsSearchtimer; }
	void SetUseAsSearchTimer(int useAsSearchtimer) { m_useAsSearchtimer = useAsSearchtimer; }
	bool UseDuration() const { return m_useDuration; }
	void SetUseDuration(bool useDuration) { m_useDuration = useDuration; }
	int MinDuration() const { return m_minDuration; }
	void SetMinDuration(int minDuration) { m_minDuration = minDuration; }
	int MaxDuration() const { return m_maxDuration; }
	void SetMaxDuration(int maxDuration) { m_maxDuration = maxDuration; }
	bool UseDayOfWeek() const { return m_useDayOfWeek; }
	void SetUseDayOfWeek(bool useDayOfWeek) { m_useDayOfWeek = useDayOfWeek; }
	int DayOfWeek() const { return m_dayOfWeek; }
	void SetDayOfWeek(int dayOfWeek) { m_dayOfWeek = dayOfWeek; }
	bool UseInFavorites() const { return m_useInFavorites; }
	void SetUseInFavorites(bool useInFavorites) { m_useInFavorites = useInFavorites; }
	int SearchTimerAction() const { return m_action; }
	void SetSearchTimerAction(int action) { m_action = action; }
	bool UseSeriesRecording() const { return  m_useEpisode; }
	void SetUseSeriesRecording(bool useEpisode) { m_useEpisode = useEpisode; }
	std::string const& Directory() const { return m_directory; }
	void SetDirectory(std::string const& directory) { m_directory = directory; }
	int DelRecsAfterDays() const { return m_delAfterDays; }
	void SetDelRecsAfterDays(int delAfterDays) { m_delAfterDays = delAfterDays; }
	int KeepRecs() const { return m_recordingsKeep; }
	void SetKeepRecs(int recordingsKeep) { m_recordingsKeep = recordingsKeep; }
	int PauseOnRecs() const {return m_pauseOnNrRecordings; }
	void SetPauseOnRecs(int pauseOnNrRecordings) { m_pauseOnNrRecordings = pauseOnNrRecordings; }
	int BlacklistMode() const {return m_blacklistmode; }
	void SetBlacklistMode(int blacklistmode) { m_blacklistmode = blacklistmode; }
	bool BlacklistSelected(int id) const;
	void ParseBlacklist( std::string const& data );
	int SwitchMinBefore() const { return m_switchMinBefore; }
	void SetSwitchMinBefore(int switchMinBefore) { m_switchMinBefore = switchMinBefore; }
	bool UseExtEPGInfo() const { return m_useExtEPGInfo; }
	void SetUseExtEPGInfo(bool useExtEPGInfo) { m_useExtEPGInfo = useExtEPGInfo; }
	std::vector< std::string > ExtEPGInfo() const { return m_ExtEPGInfo; } 
	void SetExtEPGInfo(std::vector< std::string > ExtEPGInfo) { m_ExtEPGInfo = ExtEPGInfo; } 
	bool AvoidRepeats() const { return m_avoidrepeats; }
	void SetAvoidRepeats(bool avoidrepeats) { m_avoidrepeats = avoidrepeats; }
	int AllowedRepeats() const { return m_allowedrepeats; }
	void SetAllowedRepeats(int allowedrepeats) { m_allowedrepeats = allowedrepeats; }
	int RepeatsWithinDays() const { return m_repeatsWithinDays; }
	void SetRepeatsWithinDays(int repeatsWithinDays) { m_repeatsWithinDays = repeatsWithinDays; }
	bool CompareTitle() const { return m_compareTitle; }
	void SetCompareTitle(bool compareTitle) { m_compareTitle = compareTitle; }
	int CompareSubtitle() const { return m_compareSubtitle; }
	void SetCompareSubtitle(int compareSubtitle) { m_compareSubtitle = compareSubtitle; }
	bool CompareSummary() const { return m_compareSummary; }
	void SetCompareSummary(bool compareSummary) { m_compareSummary = compareSummary; }
	unsigned long CompareCategories() const { return m_catvaluesAvoidRepeat; }
	void SetCompareCategories(unsigned long compareCategories) { m_catvaluesAvoidRepeat = compareCategories; }
	int Priority() const { return m_priority; }
	void SetPriority(int priority) { m_priority = priority; }
	int Lifetime() const { return m_lifetime; }
	void SetLifetime(int lifetime) { m_lifetime = lifetime; }
	int MarginStart() const { return m_marginstart; }
	void SetMarginStart(int marginstart) { m_marginstart = marginstart; }
	int MarginStop() const { return m_marginstop; }
	void SetMarginStop(int marginstop) { m_marginstop = marginstop; }
	bool UseVPS() const { return m_useVPS; }
	void SetUseVPS(bool useVPS) { m_useVPS = useVPS; }
	int DelMode() const { return m_delMode; }
	void SetDelMode(int delMode) { m_delMode = delMode; }
	int DelAfterCountRecs() const { return m_delAfterCountRecs; }
	void SetDelAfterCountRecs(int delAfterCountRecs) { m_delAfterCountRecs = delAfterCountRecs; }
	int DelAfterDaysOfFirstRec() const { return m_delAfterDaysOfFirstRec; }
	void SetDelAfterDaysOfFirstRec(int delAfterDaysOfFirstRec) { m_delAfterDaysOfFirstRec = delAfterDaysOfFirstRec; }
	std::string UseAsSearchTimerFrom(std::string const& format);
	void SetUseAsSearchTimerFrom(std::string const& datestring, std::string const& format);
	std::string UseAsSearchTimerTil(std::string const& format);
	void SetUseAsSearchTimerTil(std::string const& datestring, std::string const& format);
	bool IgnoreMissingEPGCats() const { return m_ignoreMissingEPGCats; }
	void SetIgnoreMissingEPGCats(bool ignoreMissingEPGCats) { m_ignoreMissingEPGCats = ignoreMissingEPGCats; }
	
private:
	int m_id;
	std::string m_search;
	bool m_useTime;
	int m_startTime;
	int m_stopTime;
	int m_useChannel;
	tChannelID m_channelMin;
	tChannelID m_channelMax;
	std::string m_channels;
	bool m_useCase;
	int m_mode;
	bool m_useTitle;
	bool m_useSubtitle;
	bool m_useDescription;
	bool m_useDuration;
	int m_minDuration;
	int m_maxDuration;
	bool m_useDayOfWeek;
	int m_dayOfWeek;
	bool m_useEpisode;
	int m_priority;
	int m_lifetime;
	int m_fuzzytolerance;
	bool m_useInFavorites;
	int m_useAsSearchtimer;
	int m_action;
	std::string m_directory;
	int m_delAfterDays;
	int m_recordingsKeep;
	int m_pauseOnNrRecordings;
	int m_switchMinBefore;
	int m_marginstart;
	int m_marginstop;
	bool m_useVPS;
	bool m_useExtEPGInfo;
	std::vector< std::string > m_ExtEPGInfo;
	bool m_avoidrepeats;
	int m_allowedrepeats;
	bool m_compareTitle;
	int m_compareSubtitle;
	bool m_compareSummary;
	int m_repeatsWithinDays;
	int m_blacklistmode;
	std::vector< std::string > m_blacklistIDs;
	int m_menuTemplate;
	unsigned long m_catvaluesAvoidRepeat;
	int m_delMode;
	int m_delAfterCountRecs;
	int m_delAfterDaysOfFirstRec;
	time_t m_useAsSearchTimerFrom;
	time_t m_useAsSearchTimerTil;
	bool m_ignoreMissingEPGCats;

	void ParseChannel( std::string const& data );
	void ParseChannelIDs( std::string const& data );
	void ParseExtEPGInfo( std::string const& data );
};

class ExtEPGInfo
{
public:
	ExtEPGInfo(std::string const& data );
	int Id() const { return m_id; }
	std::string Name() const { return m_menuname; }
	std::vector< std::string > Values() const { return m_values; }
	bool Selected(unsigned int index, std::string const& values);
private:
	int m_id;
	std::string m_name;
	std::string m_menuname;
	std::vector< std::string > m_values;
	int m_searchmode;

	void ParseValues( std::string const& data );
};

class ExtEPGInfos
{
public:
	typedef std::list< ExtEPGInfo > ExtEPGInfoList;
	typedef ExtEPGInfoList::size_type size_type;
	typedef ExtEPGInfoList::iterator iterator;
	typedef ExtEPGInfoList::const_iterator const_iterator;

	ExtEPGInfos();

	size_type size() const { return m_infos.size(); }

	iterator begin() { return m_infos.begin(); }
	const_iterator begin() const { return m_infos.begin(); }
	iterator end() { return m_infos.end(); }
	const_iterator end() const { return m_infos.end(); }
private:
	ExtEPGInfoList m_infos;
};

class ChannelGroup
{
public:
	ChannelGroup(std::string const& data );
	std::string Name() { return m_name; }
private:
	std::string m_name;
};

class ChannelGroups
{
public:
	typedef std::list< ChannelGroup > ChannelGroupList;
	typedef ChannelGroupList::size_type size_type;
	typedef ChannelGroupList::iterator iterator;
	typedef ChannelGroupList::const_iterator const_iterator;

	ChannelGroups();

	size_type size() const { return m_list.size(); }

	iterator begin() { return m_list.begin(); }
	const_iterator begin() const { return m_list.begin(); }
	iterator end() { return m_list.end(); }
	const_iterator end() const { return m_list.end(); }
private:
	ChannelGroupList m_list;
};

class SearchTimers
{
public:
	typedef std::list< SearchTimer > TimerList;
	typedef TimerList::size_type size_type;
	typedef TimerList::iterator iterator;
	typedef TimerList::const_iterator const_iterator;

	SearchTimers();
	bool Save(SearchTimer* searchtimer);
	bool Reload();

	size_type size() const { return m_timers.size(); }

	iterator begin() { return m_timers.begin(); }
	const_iterator begin() const { return m_timers.begin(); }
	iterator end() { return m_timers.end(); }
	const_iterator end() const { return m_timers.end(); }
	SearchTimer* GetByTimerId( std::string const& id );
	bool ToggleActive(std::string const& id);
	bool Delete(std::string const& id);
	void TriggerUpdate();
private:
	TimerList m_timers;
};

class Blacklist
{
public:
	Blacklist( std::string const& data );

	std::string const& Search() const { return m_search; }
	int Id() const { return m_id; }
	bool operator<( Blacklist const& other ) const { return Search() < other.Search(); }

private:
	int m_id;
	std::string m_search;
};

class Blacklists
{
public:
	typedef std::list< Blacklist > blacklist;
	typedef blacklist::size_type size_type;
	typedef blacklist::iterator iterator;
	typedef blacklist::const_iterator const_iterator;

	Blacklists();

	size_type size() const { return m_list.size(); }

	iterator begin() { return m_list.begin(); }
	const_iterator begin() const { return m_list.begin(); }
	iterator end() { return m_list.end(); }
	const_iterator end() const { return m_list.end(); }
private:
	blacklist m_list;
};

class SearchResult
{
public:
	SearchResult( std::string const& data );

	int SearchId() const { return m_searchId; }
	tEventID EventId() const { return m_eventId; }
	std::string const& Title() const { return m_title; }
	std::string const& ShortText() const { return m_shorttext; }
	std::string const& Description() const { return m_description; }
	time_t StartTime() const { return m_starttime; }
	time_t StopTime() const { return m_stoptime; }
	tChannelID Channel() const { return m_channel; }
	time_t TimerStartTime() const { return m_timerstart; }
	time_t TimerStopTime() const { return m_timerstop; }
	int TimerMode() const { return m_timerMode; }
	bool operator<( SearchResult const& other ) const { return m_starttime <  other.m_starttime; }
	const cEvent* GetEvent();
	const cChannel* GetChannel() { return Channels.GetByChannelID(m_channel); }

private:
	int m_searchId;
	tEventID m_eventId;
	std::string m_title;
	std::string m_shorttext;
	std::string m_description;
	time_t m_starttime;
	time_t m_stoptime;
	tChannelID m_channel;
	time_t m_timerstart;
	time_t m_timerstop;
	std::string m_file;
	int m_timerMode;
};

class SearchResults
{
	static std::set<std::string> querySet;
public:
	typedef std::list< SearchResult > searchresults;
	typedef searchresults::size_type size_type;
	typedef searchresults::iterator iterator;
	typedef searchresults::const_iterator const_iterator;

	SearchResults() {}
	void GetByID(int id);
	void GetByQuery(std::string const& query);

	size_type size() const { return m_list.size(); }

	iterator begin() { return m_list.begin(); }
	const_iterator begin() const { return m_list.begin(); }
	iterator end() { return m_list.end(); }
	const_iterator end() const { return m_list.end(); }

	void merge(SearchResults& r) {m_list.merge(r.m_list); m_list.sort();}
	static std::string AddQuery(std::string const& query);
	static std::string PopQuery(std::string const& md5);
private:
	searchresults m_list;
};

class RecordingDirs
{
public:
	typedef std::set< std::string > recordingdirs;
	typedef recordingdirs::size_type size_type;
	typedef recordingdirs::iterator iterator;
	typedef recordingdirs::const_iterator const_iterator;
	
	RecordingDirs(bool shortList=false);

	iterator begin() { return m_set.begin(); }
	const_iterator begin() const { return m_set.begin(); }
	iterator end() { return m_set.end(); }
	const_iterator end() const { return m_set.end(); }

private:
	recordingdirs m_set;
};

class EPGSearchSetupValues
{
public:
	static std::string ReadValue(const std::string& entry);	
	static bool WriteValue(const std::string& entry, const std::string& value);	
};

class EPGSearchExpr
{
public:
  static std::string EvaluateExpr(const std::string& expr, const cEvent* event);
};

}

 // namespace vdrlive

#endif // VDR_LIVE_EPGSEARCH_H
