
#include "thread.h"

#include "tntconfig.h"

#include <vdr/tools.h>

namespace vdrlive {

using namespace tnt;


ServerThread::ServerThread()
{
}

ServerThread::~ServerThread()
{
	Stop();
}

void ServerThread::Stop()
{
	if ( Active() ) {
		m_server->shutdown();
		Cancel( 5 );
	}
}

void ServerThread::Action()
{
	try {
		m_server.reset(new Tntnet());
		TntConfig::Get().Configure(*m_server);
		m_server->run();
		m_server.reset(0);
	} catch (std::exception const& ex) {
		// XXX move initial error handling to live.cpp
		esyslog("live: ERROR: live httpd server crashed: %s", ex.what());
		std::cerr << "HTTPD FATAL ERROR: " << ex.what() << std::endl;
		//cThread::EmergencyExit(true);
	}
}

} // namespace vdrlive
