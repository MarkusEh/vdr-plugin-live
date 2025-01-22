#include "tasks.h"
#include "stdext.h"
#include "recman.h"

// STL headers need to be before VDR tools.h (included by <vdr/menu.h>)
#include <algorithm>
#include <functional>

#include <vdr/menu.h>

namespace vdrlive {

const char* NowReplaying()
{
  return cReplayControl::NowReplaying();
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
  LOCK_CHANNELS_READ;
  cChannel* channel = (cChannel *)Channels->GetByChannelID( m_channel );
  if ( channel == 0 ) {
    SetError( tr("Couldn't find channel or no channels available.") );
    return;
  }

  if ( !Channels->SwitchTo( channel->Number() ) )
    SetError( tr("Couldn't switch to channel.") );
}

void PlayRecordingTask::Action()
{
  bool this_is_not_current;
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
    if ( recording == 0 ) {
      SetError( tr("Couldn't find recording or no recordings available.") );
      return;
    }
    const char *current = NowReplaying();
    this_is_not_current = !current || (0 != strcmp(current, recording->FileName()));
    if (this_is_not_current) {
      cReplayControl::SetRecording( 0 );
      cControl::Shutdown();
      cReplayControl::SetRecording( recording->FileName() );
      cControl::Launch( new cReplayControl );
      cControl::Attach();
    }
  }
  if (!this_is_not_current) {
#if APIVERSNUM >= 20402
    cMutexLock mutexLock;
    cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control(mutexLock));
#else
    cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
#endif
    if (! replayControl) {
      SetError(tr("Cannot control playback!"));
      return;
    }

    replayControl->Play();
  }
}

void PauseRecordingTask::Action()
{
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
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
  }

#if APIVERSNUM >= 20402
  cMutexLock mutexLock;
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control(mutexLock));
#else
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
#endif
  if (! replayControl) {
    SetError(tr("Cannot control playback!"));
    return;
  }

  replayControl->Pause();
}

void StopRecordingTask::Action()
{
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
    if ( recording == 0 ) {
      SetError( tr("Couldn't find recording or no recordings available.") );
      return;
    }
  }
  const char *current = NowReplaying();
  if (!current) {
    SetError(tr("Not playing a recording."));
    return;
  }

  cReplayControl::SetRecording( 0 );
  cControl::Shutdown();
}

void ForwardRecordingTask::Action()
{
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
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
  }

#if APIVERSNUM >= 20402
  cMutexLock mutexLock;
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control(mutexLock));
#else
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
#endif
  if (! replayControl) {
    SetError(tr("Cannot control playback!"));
    return;
  }

  replayControl->Forward();
}

void BackwardRecordingTask::Action()
{
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
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
  }

#if APIVERSNUM >= 20402
  cMutexLock mutexLock;
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control(mutexLock));
#else
  cReplayControl* replayControl = reinterpret_cast<cReplayControl*>(cControl::Control());
#endif
  if (! replayControl) {
    SetError(tr("Cannot control playback!"));
    return;
  }

  replayControl->Backward();
}


void RemoveRecordingTask::Action()
{
  {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = RecordingsManager::GetByHash(m_recording, Recordings);
    if (!recording) {
      SetError( tr("Couldn't find recording or no recordings available.") );
      return;
    }

    m_recName = recording->Name();

    const char *current = NowReplaying();
    if (current && (0 == strcmp(current, recording->FileName()))) {
      SetError(tr("Attempt to delete recording currently in playback."));
      return;
    }
  }

  RecordingsManager::DeleteRecording(m_recording);
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

   using std::for_each;
   using std::bind;
   using namespace std::placeholders;

  cMutexLock lock( this );
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
