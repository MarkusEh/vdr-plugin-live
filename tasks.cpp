#include <vdr/channels.h>
#include <vdr/i18n.h>
#include <vdr/menu.h>
#include <vdr/recording.h>
#include "exception.h"
#include "recordings.h"
#include "tasks.h"
#include "tools.h"

using namespace std;

namespace vdrlive {

void SwitchChannelTask::Action() 
{
	ReadLock lock( Channels );
	cChannel* channel = Channels.GetByChannelID( m_channel );
	if ( channel == 0 ) {
		SetError( tr("Couldn't find channel or no channels available.") );
		return;
	}

	if ( !Channels.SwitchTo( channel->Number() ) )
		SetError( tr("Couldn't switch to channel.") );
}

void ReplayRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	cReplayControl::SetRecording( 0, 0 );
	cControl::Shutdown();
	cReplayControl::SetRecording( recording->FileName(), recording->Title() );
	cControl::Launch( new cReplayControl );
	cControl::Attach();
}

TaskManager::TaskManager()
{
}

bool TaskManager::Execute( Task& task )
{
	cMutexLock lock( this );

	m_taskQueue.push_back( &task );
	m_scheduleWait.Wait( *this );
	return task.Result();
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

TaskManager& LiveTaskManager()
{
	static TaskManager instance;
	return instance;
}

} // namespace vdrlive
