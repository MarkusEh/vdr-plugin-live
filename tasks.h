#ifndef VDR_LIVE_TASKS_H
#define VDR_LIVE_TASKS_H

#include <deque>
#include <memory>
#include <string>
#include <vdr/channels.h>
#include <vdr/thread.h>

namespace vdrlive {

class Task;

class TaskManager: public cMutex
{
	friend TaskManager& LiveTaskManager();

	typedef std::deque< Task* > TaskQueue;

public:
	bool Execute( Task* task, std::string& error );
	bool Execute( Task* task );

	// may only be called from Plugin::MainThreadHook
	void DoScheduledTasks();

private:
	TaskManager();
	TaskManager( TaskManager const& );

	TaskQueue m_taskQueue;
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
	Task(): m_result( true ) {}
	Task( Task const& );

	void SetError( std::string const& error ) { m_result = false; m_error = error; }

	virtual void Action() = 0;

private:
	bool m_result;
	std::string m_error;
};

class SwitchChannelTask: public Task
{
public:
	SwitchChannelTask( tChannelID channel ): m_channel( channel ) {}
	
private:
	tChannelID m_channel;

	virtual void Action();
};

class ReplayRecordingTask: public Task
{
public:
	ReplayRecordingTask( std::string const& recording ): m_recording( recording ) {}

private:
	std::string m_recording;
	
	virtual void Action() {}
};


TaskManager& LiveTaskManager();

} // namespace vdrlive

#endif // VDR_LIVE_TASKS_H
