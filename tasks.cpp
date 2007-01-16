#include <vdr/channels.h>
#include <vdr/i18n.h>
#include <vdr/menu.h>
#include <vdr/recording.h>
#include "exception.h"
#include "tasks.h"
#include "tools.h"

using namespace std;

namespace vdrlive {

void SwitchChannelTask::Action() 
{
	ReadLock lock( Channels );
	cChannel* channel = Channels.GetByChannelID( m_channel );
	if ( channel == 0 ) {
		SetError( tr("Couldn't find channel or no channels available. Maybe you mistyped your request?") );
		return;
	}

	if ( !Channels.SwitchTo( channel->Number() ) )
		SetError( tr("Couldn't switch channels") );
}

TaskManager::TaskManager()
{
}

bool TaskManager::Execute( Task* task, string& error )
{
	auto_ptr< Task > reaper( task );
	cMutexLock lock( this );

	m_taskQueue.push_back( task );
	m_scheduleWait.Wait( *this );
	error = task->Error();
	return task->Result();
}

bool TaskManager::Execute( Task* task )
{
	string dummyError;
	return Execute( task, dummyError );
}

void TaskManager::DoScheduledTasks()
{
	if ( m_taskQueue.empty() )
		return;

	cMutexLock lock( this );
	while ( !m_taskQueue.empty() ) {
		Task* current = m_taskQueue.front();
		current->Action();
		m_taskQueue.pop_front();
	}
	m_scheduleWait.Broadcast();
}

/*
bool TaskManager::ReplayRecording( std::string const& fileName )
{
	return ScheduleCommand( m_replayRecording, fileName );
	//cMutexLock lock( this );
	//m_replayFileName = fileName;
	//m_scheduleWait.Wait( *this );
	//return m_replayResult;
}
*/

TaskManager& LiveTaskManager()
{
	static TaskManager instance;
	return instance;
}

} // namespace vdrlive
