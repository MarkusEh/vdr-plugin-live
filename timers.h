#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

#include <list>
#include <string>
#include <vdr/timers.h>
#include <vdr/thread.h>
#include "live.h"

namespace vdrlive {

class SortedTimers: public std::list< cTimer >
{
	friend class TimerManager;

public:
	std::string GetTimerId( cTimer const& timer );
	cTimer* GetByTimerId( std::string const& timerid );
	
private:
	SortedTimers();
	SortedTimers( SortedTimers const& );

	int m_state;
	
	void ReloadTimers( bool initial = false );
};

class TimerManager: public cMutex
{
	friend TimerManager& LiveTimerManager();

public:
	SortedTimers& GetTimers() { return m_timers; }

	// may only be called from Plugin::MainThreadHook
	void DoPendingWork();

private:
	TimerManager();
	TimerManager( TimerManager const& );

	SortedTimers m_timers;
};

TimerManager& LiveTimerManager();

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
