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

#ifndef TNTVERS7
class ProtectedCString
{
public:
	ProtectedCString( char const* string ): m_string( strdup( string ) ) {}
	~ProtectedCString() { free( m_string ); }

	operator char*() { return m_string; }

private:
	char* m_string;
};
#endif // TNTVERS7

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
#ifdef TNTVERS7
		tnt::Tntconfig tntconfig;
		tntconfig.load(TntConfig::Get().GetConfigPath().c_str());
		m_server.reset(new Tntnet());
		m_server->init(tntconfig);
#else
		ProtectedCString configPath( TntConfig::Get().GetConfigPath().c_str() );

		char* argv[] = { const_cast< char* >( "tntnet" ), const_cast< char* >( "-c" ), configPath };
		int argc = sizeof( argv ) / sizeof( argv[0] );

		m_server.reset( new Tntnet( argc, argv ) );
#endif // TNTVERS7
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
