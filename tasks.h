#ifndef VDR_LIVE_TASKS_H
#define VDR_LIVE_TASKS_H

#include <vdr/thread.h>

namespace vdrlive {

class TaskManager: public cMutex
{
	friend TaskManager& LiveTaskManager();

public:
	bool SwitchChannel( int number );

	// may only be called from Plugin::MainThreadHook
	void DoScheduledWork();
	
private:
	TaskManager();
	TaskManager( TaskManager const& );

	int m_switchChannel;
	bool m_switchResult;
	cCondVar m_scheduleWait;
};

TaskManager& LiveTaskManager();

} // namespace vdrlive

#endif // VDR_LIVE_TASKS_H
