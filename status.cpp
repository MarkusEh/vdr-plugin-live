
#include "status.h"

#include "timers.h"

namespace vdrlive {

StatusMonitor::StatusMonitor()
{
}

void StatusMonitor::TimerChange(const cTimer *Timer, eTimerChange Change)
{
//  LiveTimerManager().SetReloadTimers();
}

void StatusMonitor::Recording( cDevice const*, char const*, char const*, bool )
{
//  LiveTimerManager().SetReloadTimers();
}

StatusMonitor& LiveStatusMonitor()
{
  static StatusMonitor instance;
  return instance;
}

} // namespace vdrlive
