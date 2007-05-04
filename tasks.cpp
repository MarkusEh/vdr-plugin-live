#include <algorithm>
#include <boost/bind.hpp>
#include <vdr/channels.h>
#include <vdr/i18n.h>
#include <vdr/menu.h>
#include <vdr/recording.h>
#include "exception.h"
#include "recordings.h"
#include "tasks.h"
#include "tools.h"

namespace vdrlive {

using namespace std;
using namespace boost;

StickyTask::StickyTask()
{
	LiveTaskManager().AddStickyTask( *this );
}

StickyTask::~StickyTask()
{
	LiveTaskManager().RemoveStickyTask( *this );
}

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

void TaskManager::AddStickyTask( Task& task ) 
{ 
	cMutexLock lock( this );
	m_stickyTasks.push_back( &task ); 
}
	
void TaskManager::RemoveStickyTask( Task& task )
{
	cMutexLock lock( this );
	m_stickyTasks.erase( find( m_stickyTasks.begin(), m_stickyTasks.end(), &task ) );
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
	if ( m_taskQueue.empty() && m_stickyTasks.empty() )
		return;

	cMutexLock lock( this );
	/*while ( !m_taskQueue.empty() ) {
		Task* current = m_taskQueue.front();
		current->Action();
		m_taskQueue.pop_front();
	}*/
	for_each( m_taskQueue.begin(), m_taskQueue.end(), bind( &Task::Action, _1 ) );
	for_each( m_stickyTasks.begin(), m_stickyTasks.end(), bind( &Task::Action, _1 ) );
	m_taskQueue.clear();
	m_scheduleWait.Broadcast();
}

TaskManager& LiveTaskManager()
{
	static TaskManager instance;
	return instance;
}

} // namespace vdrlive
