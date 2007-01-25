#ifndef VDR_LIVE_EPGSEARCH_H
#define VDR_LIVE_EPGSEARCH_H

#include <list>
#include <string>
#include <vdr/channels.h>

namespace vdrlive {

class SearchTimer;

bool operator<( SearchTimer const& left, SearchTimer const& right );

class SearchTimer
{
public:
	enum UseChannel
	{
		NoChannel = 0,
		Interval = 1,
		Group = 2,
		FTAOnly = 3
	};
	
	SearchTimer( std::string const& data );

	int Id() const { return m_id; }
	std::string const& Search() const { return m_search; }
	bool UseTime() const { return m_useTime; }
	int StartTime() const { return m_startTime; }
	int StopTime() const { return m_stopTime; }
	bool UseChannel() const { return m_useChannel; }
	tChannelID const& ChannelMin() const { return m_channelMin; }
	tChannelID const& ChannelMax() const { return m_channelMax; }
	bool UseAsSearchTimer() const { return m_useAsSearchTimer; }

	friend bool operator<( SearchTimer const& left, SearchTimer const& right );

private:
	int m_id;
	std::string m_search;
	bool m_useTime;
	int m_startTime;
	int m_stopTime;
	int m_useChannel;
	tChannelID m_channelMin;
	tChannelID m_channelMax;
	std::string m_channelGroup;
	bool m_useCase;
	int m_mode;
	bool m_useTitle;
	bool m_useSubtitle;
	bool m_useDescription;
	bool m_useDuration;
	int m_minDuration;
	int m_maxDuration;
	bool m_useAsSearchTimer;
	bool m_useDayOfWeek;
	int m_dayOfWeek;
	bool m_useEpisode;
	std::string m_directory;
	int m_priority;
	int m_lifetime;

	void ParseChannel( std::string const& data );
	void ParseChannelIDs( std::string const& data );
/*
  int      useChannel;
  int      useCase;
  int      mode;
  int      useTitle;
  int      useSubtitle;
  int      useDescription;
  int      useDuration;
  int      minDuration;
  int      maxDuration;
  int      useAsSearchTimer;
  int      useDayOfWeek;
  int      DayOfWeek;
  int      useEpisode;
  char     directory[MaxFileName];
  int      Priority;
  int      Lifetime;
  int      MarginStart;
  int      MarginStop;
  int      useVPS;
  int      action;
  int      useExtEPGInfo;
  char**   catvalues;
  cChannel *channelMin;
  cChannel *channelMax;
  char*    channelGroup;
  int      avoidRepeats;
  int      compareTitle;
  int      compareSubtitle;
  int      compareSummary;
  int      allowedRepeats;
  unsigned long catvaluesAvoidRepeat;
  int      repeatsWithinDays;
  int      delAfterDays;
  int      recordingsKeep;
  int      switchMinsBefore;
  int      pauseOnNrRecordings;
  int      blacklistMode;
  cList<cBlacklistObject> blacklists;
  int      fuzzyTolerance;
  int      useInFavorites;
  int      menuTemplate;*/
};

class SearchTimers
{
public:
	typedef std::list< SearchTimer > TimerList;
	typedef TimerList::size_type size_type;
	typedef TimerList::iterator iterator;
	typedef TimerList::const_iterator const_iterator;

	SearchTimers();

	size_type size() const { return m_timers.size(); }

	iterator begin() { return m_timers.begin(); }
	const_iterator begin() const { return m_timers.begin(); }
	iterator end() { return m_timers.end(); }
	const_iterator end() const { return m_timers.end(); }

private:
	TimerList m_timers;
};


} // namespace vdrlive

#endif // VDR_LIVE_EPGSEARCH_H
