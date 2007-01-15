#include <vdr/channels.h>
#include <vdr/menu.h>
#include <vdr/recording.h>
#include "tasks.h"

namespace vdrlive {

TaskManager::TaskManager():
		m_switchChannel( 0, false ),
		m_replayRecording( "", false )
{
}

bool TaskManager::SwitchChannel( int number )
{
	return ScheduleCommand( m_switchChannel, number );
	//cMutexLock lock( this );
	//m_switchChannel.first = number;
	//m_scheduleWait.Wait( *this );
	//return m_switchChannel.second;
}

bool TaskManager::ReplayRecording( std::string const& fileName )
{
	return ScheduleCommand( m_replayRecording, fileName );
	//cMutexLock lock( this );
	//m_replayFileName = fileName;
	//m_scheduleWait.Wait( *this );
	//return m_replayResult;
}

void TaskManager::DoScheduledWork()
{
	if ( m_switchChannel.first == 0 && m_replayRecording.first.empty() )
		return;

	cMutexLock lock( this );
	if ( m_switchChannel.first != 0 )
		DoSwitchChannel();
	if ( !m_replayRecording.first.empty() )
		DoReplayRecording();
	m_scheduleWait.Broadcast();
}

void TaskManager::DoSwitchChannel()
{
	m_switchChannel.second = Channels.SwitchTo( m_switchChannel.first );
	m_switchChannel.first = 0;
}

void TaskManager::DoReplayRecording()
{
	bool result = false;
	cThreadLock lock( &Recordings );

	cRecording* recording = Recordings.GetByName( m_replayRecording.first.c_str() );
	if ( recording ) {
		cReplayControl::SetRecording( 0, 0 );
		cControl::Shutdown();
		cReplayControl::SetRecording( recording->FileName(), recording->Title() );
		cControl::Launch( new cReplayControl );
		cControl::Attach();
		result = true;
	}
	m_replayRecording.first.clear();
}

TaskManager& LiveTaskManager()
{
	static TaskManager instance;
	return instance;
}

} // namespace vdrlive
