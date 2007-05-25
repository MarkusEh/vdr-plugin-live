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

class ProtectedCString
{
public:
	ProtectedCString( char const* string ): m_string( strdup( string ) ) {}
	~ProtectedCString() { free( m_string ); }

	operator char*() { return m_string; }

private:
	char* m_string;
};

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
		ProtectedCString configPath( TntConfig::Get().GetConfigPath().c_str() );

		char* argv[] = { "tntnet", "-c", configPath };
		int argc = sizeof( argv ) / sizeof( argv[0] );
		m_server.reset( new Tntnet( argc, argv ) );
		m_server->run();
		m_server.reset( 0 );
	} catch ( exception const& ex ) {
		// XXX move initial error handling to live.cpp
		esyslog( "ERROR: live httpd server crashed: %s", ex.what() );
		cerr << "HTTPD FATAL ERROR: " << ex.what() << endl;
		//cThread::EmergencyExit(true);
	}
}

} // namespace vdrlive
