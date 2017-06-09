
#include "status.h"

#include "timers.h"

namespace vdrlive {

StatusMonitor::StatusMonitor()
{
}

void StatusMonitor::TimerChange(const cTimer *Timer, eTimerChange Change)
{
	cMutexLock timersLock( &LiveTimerManager() );
	LiveTimerManager().SetReloadTimers();
}

void StatusMonitor::Recording( cDevice const*, char const*, char const*, bool )
{
	cMutexLock timersLock( &LiveTimerManager() );
	LiveTimerManager().DoReloadTimers();
}

StatusMonitor& LiveStatusMonitor()
{
	static StatusMonitor instance;
	return instance;
}

} // namespace vdrlive
