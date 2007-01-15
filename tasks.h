#ifndef VDR_LIVE_TASKS_H
#define VDR_LIVE_TASKS_H

#include <string>
#include <utility>
#include <vdr/thread.h>

namespace vdrlive {

class TaskManager: public cMutex
{
	friend TaskManager& LiveTaskManager();

public:
	bool SwitchChannel( int number );
	bool ReplayRecording( std::string const& fileName );

	// may only be called from Plugin::MainThreadHook
	void DoScheduledWork();
	
private:
	template< typename Type >
	struct Task: std::pair< Type, bool > 
	{
		Task( Type const& first, bool second ): std::pair< Type, bool >( first, second ) {}
	};

	TaskManager();
	TaskManager( TaskManager const& );

	Task< int > m_switchChannel;
	Task< std::string > m_replayRecording;
	cCondVar m_scheduleWait;

	template< typename Type >
	bool ScheduleCommand( Task< Type >& member, Type const& param );

	void DoSwitchChannel();
	void DoReplayRecording();
};
	
template< typename Type >
inline bool TaskManager::ScheduleCommand( Task< Type >& member, Type const& param )
{
	cMutexLock lock( this );
	member.first = param;
	m_scheduleWait.Wait( *this );
	return member.second;
}

TaskManager& LiveTaskManager();

} // namespace vdrlive

#endif // VDR_LIVE_TASKS_H
