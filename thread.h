#ifndef VDR_LIVE_THREAD_H
#define VDR_LIVE_THREAD_H

#include <vdr/thread.h>

namespace vdrlive {

class ServerThread : public cThread {
public:
	ServerThread();
	virtual ~ServerThread();

protected:
	virtual void Action();

private:
	char* m_configPath;
};

} // namespace vdrlive

#endif // VDR_LIVE_THREAD_H
