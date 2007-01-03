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

ServerThread::ServerThread():
		m_configPath( 0 )
{
}

ServerThread::~ServerThread()
{
	free( m_configPath );
}

void ServerThread::Action()
{
	try {
		m_configPath = strdup( TntConfig::Get().GetConfigPath().c_str() );

		char* argv[] = { "tntnet", "-c", m_configPath };
		int argc = sizeof( argv ) / sizeof( argv[0] );
		Tntnet app( argc, argv );
		app.run();
	} catch ( exception const& ex ) {
		// XXX move initial error handling to live.cpp
		esyslog( "ERROR: live httpd server crashed: %s", ex.what() );
		cerr << "HTTPD FATAL ERROR: " << ex.what() << endl;
	}
}

} // namespace vdrlive
