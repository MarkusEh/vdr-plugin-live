#ifndef VDR_LIVE_STATUS_H
#define VDR_LIVE_STATUS_H

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/status.h>

namespace vdrlive {

class StatusMonitor: public cStatus
{
  friend StatusMonitor& LiveStatusMonitor();

private:
  StatusMonitor();
  StatusMonitor( StatusMonitor const& );

  virtual void TimerChange(const cTimer *Timer, eTimerChange Change);
  virtual void Recording( cDevice const* Device, char const* Name, char const* FileName, bool On );
};

StatusMonitor& LiveStatusMonitor();

} // namespace vdrlive

#endif // VDR_LIVE_STATUS_H
