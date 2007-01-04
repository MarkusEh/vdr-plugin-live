#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

#include <list>
#include <vdr/timers.h>
#include "live.h"

namespace vdrlive {

class Plugin;

class TimerManager
{
	friend TimerManager& Plugin::GetLiveTimerManager();

private:
	TimerManager();
	TimerManager( TimerManager const& );

	
};

inline TimerManager& LiveTimerManager()
{
	return LivePlugin().GetLiveTimerManager();
}

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
