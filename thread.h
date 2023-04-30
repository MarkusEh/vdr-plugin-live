#ifndef VDR_LIVE_THREAD_H
#define VDR_LIVE_THREAD_H

#include <vdr/thread.h>

#include <memory>

namespace tnt { class Tntnet; }

namespace vdrlive {

class ServerThread : public cThread {
public:
	ServerThread();
	virtual ~ServerThread();

	void Stop();

protected:
	virtual void Action();

private:
	std::unique_ptr<tnt::Tntnet> m_server;
};

} // namespace vdrlive

#endif // VDR_LIVE_THREAD_H
