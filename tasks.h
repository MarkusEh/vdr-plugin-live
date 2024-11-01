#ifndef VDR_LIVE_TASKS_H
#define VDR_LIVE_TASKS_H

// STL headers need to be before VDR tools.h (included by <vdr/channels.h>)
#include <string>
#include <vector>

#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/channels.h>
#include <vdr/thread.h>
#include "timers.h"

namespace vdrlive {

class Task;

class TaskManager: public cMutex
{
  friend TaskManager& LiveTaskManager();
  friend class StickyTask;

  typedef std::vector<Task*> TaskList;

public:
  bool Execute( Task& task );

  // may only be called from Plugin::MainThreadHook
  void DoScheduledTasks();

private:
  TaskManager();
  TaskManager( TaskManager const& );

  void AddStickyTask( Task& task );
  void RemoveStickyTask( Task& task );

  TaskList m_taskQueue;
  TaskList m_stickyTasks;
  cCondVar m_scheduleWait;
};

class Task
{
  friend void TaskManager::DoScheduledTasks();

public:
  virtual ~Task() {}

  bool Result() const { return m_result; }
  std::string const& Error() const { return m_error; }

protected:
  explicit Task()
    : m_result( true )
    {}
  Task( Task const& );

  void SetError( std::string const& error ) { m_result = false; m_error = error; }

private:
  bool m_result;
  std::string m_error;

  virtual void Action() = 0;
};

class StickyTask: public Task
{
protected:
  explicit StickyTask();
  virtual ~StickyTask();
};

class SwitchChannelTask: public Task
{
public:
  explicit SwitchChannelTask( tChannelID channel ): m_channel( channel ) {}

private:
  tChannelID m_channel;

  virtual void Action() override;
};

class RecordingTask: public Task
{
protected:
  explicit RecordingTask(std::string const& recording)
    : m_recording(recording)
  {}

  std::string m_recording;
};

class PlayRecordingTask: public RecordingTask
{
public:
  explicit PlayRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;
};

class PauseRecordingTask: public RecordingTask
{
public:
  explicit PauseRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;
};

class StopRecordingTask: public RecordingTask
{
public:
  explicit StopRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;
};

class ForwardRecordingTask: public RecordingTask
{
public:
  explicit ForwardRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;
};

class BackwardRecordingTask: public RecordingTask
{
public:
  explicit BackwardRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;
};

class RemoveRecordingTask: public RecordingTask
{
public:
  explicit RemoveRecordingTask( std::string const& recording )
    : RecordingTask(recording)
  {}

  virtual void Action() override;

  std::string const & RecName() const { return m_recName; }

private:
  std::string m_recName;
};


TaskManager& LiveTaskManager();

class cLiveWorker: public cThread {
  private:
    cMutex m_mutex;
    cCondVar m_waitCondition;
  public:
    cLiveWorker() {}
    virtual ~cLiveWorker() {}
    void Stop() {
      m_waitCondition.Broadcast();  // wakeup the thread
      Cancel(10);                   // wait up to 10 seconds for thread was stopping
    }
    virtual void Action() {
      m_mutex.Lock();
      int loopSleep = 50; // do this every 50 milli seconds
      while (Running()) {
        m_waitCondition.TimedWait(m_mutex, loopSleep);
        LiveTimerManager().DoPendingWork();
        LiveTaskManager().DoScheduledTasks();
      }
    }
};

} // namespace vdrlive

#endif // VDR_LIVE_TASKS_H
