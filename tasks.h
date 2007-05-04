#ifndef VDR_LIVE_TASKS_H
#define VDR_LIVE_TASKS_H

#include <memory>
#include <string>
#include <vector>
#include <vdr/channels.h>
#include <vdr/thread.h>

namespace vdrlive {

class Task;

class TaskManager: public cMutex
{
	friend TaskManager& LiveTaskManager();
	friend class StickyTask;

	typedef std::vector< Task* > TaskList;

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

	virtual void Action();
};

class ReplayRecordingTask: public Task
{
public:
	explicit ReplayRecordingTask( std::string const& recording ): m_recording( recording ) {}

private:
	std::string m_recording;
	
	virtual void Action();
};


TaskManager& LiveTaskManager();

} // namespace vdrlive

#endif // VDR_LIVE_TASKS_H
