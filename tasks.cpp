#include <algorithm>
#include <vdr/channels.h>
#include <vdr/i18n.h>
#include <vdr/menu.h>
#include <vdr/recording.h>

#include "stdext.h"
#include "exception.h"
#include "recman.h"
#include "tools.h"

#include "tasks.h"

namespace vdrlive {

using namespace std;
using namespace std::tr1;
using namespace std::tr1::placeholders;

const char* NowReplaying()
{
#if VDRVERSNUM >= 10403
	return cReplayControl::NowReplaying();
#else
	return cControl::Control()?cReplayControl::LastReplayed():NULL;
#endif
}

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

void PlayRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	const char *current = NowReplaying();
	if (!current || (0 != strcmp(current, recording->FileName()))) {
#if VDRVERSNUM >= 10728
		cReplayControl::SetRecording( 0 );
		cControl::Shutdown();
		cReplayControl::SetRecording( recording->FileName() );
#else
		cReplayControl::SetRecording( 0, 0 );
		cControl::Shutdown();
		cReplayControl::SetRecording( recording->FileName(), recording->Title() );
#endif
		cControl::Launch( new cReplayControl );
		cControl::Attach();
	}
	else {
		cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
		if (! replayControl) {
			SetError(tr("Cannot control playback!"));
			return;
		}

		replayControl->Play();
	}
}

void PauseRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	const char *current = NowReplaying();
	if (!current) {
		SetError(tr("Not playing a recording."));
		return;
	}

	if (0 != strcmp(current, recording->FileName())) {
		// not replaying same recording like in request
		SetError(tr("Not playing the same recording as from request."));
		return;
	}

	cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
	if (! replayControl) {
		SetError(tr("Cannot control playback!"));
		return;
	}

	replayControl->Pause();
}

void StopRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	const char *current = NowReplaying();
	if (!current) {
		SetError(tr("Not playing a recording."));
		return;
	}

#if VDRVERSNUM >= 10728
	cReplayControl::SetRecording( 0 );
#else
	cReplayControl::SetRecording( 0, 0 );
#endif
	cControl::Shutdown();
}

void ForwardRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	const char *current = NowReplaying();
	if (!current) {
		SetError(tr("Not playing a recording."));
		return;
	}

	if (0 != strcmp(current, recording->FileName())) {
		// not replaying same recording like in request
		SetError(tr("Not playing the same recording as from request."));
		return;
	}

	cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
	if (! replayControl) {
		SetError(tr("Cannot control playback!"));
		return;
	}

	replayControl->Forward();
}

void BackwardRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const* recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError(tr("Couldn't find recording or no recordings available."));
		return;
	}

	const char *current = NowReplaying();
	if (!current) {
		SetError(tr("Not playing a recording."));
		return;
	}

	if (0 != strcmp(current, recording->FileName())) {
		// not replaying same recording like in request
		SetError(tr("Not playing the same recording as from request."));
		return;
	}

	cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
	if (! replayControl) {
		SetError(tr("Cannot control playback!"));
		return;
	}

	replayControl->Backward();
}


void RemoveRecordingTask::Action()
{
	RecordingsManagerPtr recordings = LiveRecordingsManager();
	cRecording const * recording = recordings->GetByMd5Hash( m_recording );
	if ( recording == 0 ) {
		SetError( tr("Couldn't find recording or no recordings available.") );
		return;
	}

	m_recName = recording->Name();

	const char *current = NowReplaying();
	if (current && (0 == strcmp(current, recording->FileName()))) {
		SetError(tr("Attempt to delete recording currently in playback."));
		return;
	}

	recordings->DeleteRecording(recording);
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
