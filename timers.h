#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

#include <list>
#include <string>
#include <vdr/timers.h>
#include <vdr/thread.h>
#include "live.h"

namespace vdrlive {

class Plugin;

class SortedTimersInterface: public std::list< cTimer >
{
public:
	virtual ~SortedTimersInterface() {}

	virtual std::string GetTimerId( cTimer const& timer ) = 0;
};

class SortedTimers: public SortedTimersInterface
{
	friend class TimerManager;

public:
	virtual std::string GetTimerId( cTimer const& timer );
	
private:
	SortedTimers();
	SortedTimers( SortedTimers const& );

	int m_state;
	int m_refs;
	
	void ReloadTimers( bool initial = false );
};

class TimerManager: public cMutex
{
	friend TimerManager& Plugin::GetLiveTimerManager();

public:
	SortedTimersInterface& GetTimers() { return m_timers; }

	// may only be called from Plugin::MainThreadHook
	void DoPendingWork();

private:
	TimerManager();
	TimerManager( TimerManager const& );

	SortedTimers m_timers;
};

inline TimerManager& LiveTimerManager()
{
	return LivePlugin().GetLiveTimerManager();
}

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
