#ifndef VDR_LIVE_STATUS_H
#define VDR_LIVE_STATUS_H

#include <vdr/status.h>

namespace vdrlive {

class StatusMonitor: public cStatus
{
	friend StatusMonitor& LiveStatusMonitor();

private:
	StatusMonitor();
	StatusMonitor( StatusMonitor const& );
 
	virtual void Recording( cDevice const* Device, char const* Name, char const* FileName, bool On );
};

StatusMonitor& LiveStatusMonitor();

} // namespace vdrlive

#endif // VDR_LIVE_STATUS_H
