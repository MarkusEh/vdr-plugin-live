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

	std::string UpdateTimer( cTimer* timer, std::string const& timerString );

	// may only be called from Plugin::MainThreadHook
	void DoPendingWork();

private:
	typedef std::pair< cTimer*, std::string > TimerPair;
	typedef std::pair< TimerPair, std::string > ErrorPair;
	typedef std::list< TimerPair > TimerList;
	typedef std::list< ErrorPair > ErrorList;
	
	TimerManager();
	TimerManager( TimerManager const& );

	SortedTimers m_timers;
	TimerList m_updateTimers;
	ErrorList m_failedUpdates;
	cCondVar m_updateWait;

	void DoUpdateTimers();
	void DoInsertTimer( TimerPair& timerData );
	void DoUpdateTimer( TimerPair& timerData );

	void StoreError( TimerPair const& timerData, std::string const& error );
	std::string GetError( TimerPair const& timerData );
};

TimerManager& LiveTimerManager();

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
