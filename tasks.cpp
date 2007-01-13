#include <vdr/channels.h>
#include "tasks.h"

namespace vdrlive {

TaskManager::TaskManager():
		m_switchChannel( 0 ),
		m_switchResult( false )
{
}

bool TaskManager::SwitchChannel( int number )
{
	cMutexLock lock( this );
	m_switchChannel = number;
	m_scheduleWait.Wait( *this );
	return m_switchResult;
}

void TaskManager::DoScheduledWork()
{
	if ( m_switchChannel == 0 )
		return;

	cMutexLock lock( this );
	if ( m_switchChannel != 0 ) {
		m_switchResult = Channels.SwitchTo( m_switchChannel );
		m_switchChannel = 0;
		m_scheduleWait.Broadcast();
	}
}

TaskManager& LiveTaskManager()
{
	static TaskManager instance;
	return instance;
}

} // namespace vdrlive
