#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vdr/tools.h>
#include <tnt/tntnet.h>
#include "thread.h"
#include "tntconfig.h"

namespace vdrlive {

using namespace std;
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
	} catch (exception const& ex) {
		// XXX move initial error handling to live.cpp
		esyslog("ERROR: live httpd server crashed: %s", ex.what());
		cerr << "HTTPD FATAL ERROR: " << ex.what() << endl;
		//cThread::EmergencyExit(true);
	}
}

} // namespace vdrlive
