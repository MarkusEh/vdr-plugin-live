#include "status.h"
#include "timers.h"

namespace vdrlive {

StatusMonitor::StatusMonitor()
{
}
	
void StatusMonitor::Recording( cDevice const*, char const*, char const*, bool )
{
	LiveTimerManager().DoReloadTimers();
}

StatusMonitor& LiveStatusMonitor()
{
	static StatusMonitor instance;
	return instance;
}

} // namespace vdrlive
