#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vdr/tools.h>
#include "setup.h"

namespace vdrlive {

using namespace std;

Setup::Setup():
		m_libraryPath( "/usr/local/lib" ),
		m_serverPort( 8001 ),
		m_lastChannel( 0 )
{
}

bool Setup::ParseCommandLine( int argc, char* argv[] )
{
	static struct option opts[] = {
			{ "lib",  required_argument, NULL, 'L' },
			{ "port", required_argument, NULL, 'p' },
			{ "ip",   required_argument, NULL, 'i' },
			{ 0 }
	};

	int optchar, optind = 0;
	while ( ( optchar = getopt_long( argc, argv, "L:p:i:", opts, &optind ) ) != -1 ) {
		switch ( optchar ) {
		case 'L': m_libraryPath = optarg; break;
		case 'p': m_serverPort = atoi( optarg ); break;
		case 'i': m_serverIps.push_back( optarg ); break;
		default:  return false;
		}
	}

	return CheckLibraryPath() &&
		   CheckServerPort() &&
		   CheckServerIps();
}

char const* Setup::CommandLineHelp() const
{
	if ( m_helpString.empty() ) {
		ostringstream builder;
		builder << "  -L DIR,   --lib=DIR       libtnt-live.so will be searched in DIR\n"
				   "                            (default: " << m_libraryPath << ")\n"
				<< "  -p PORT,  --port=PORT     use PORT to listen for incoming connections\n"
				   "                            (default: " << m_serverPort << ")\n"
				<< "  -i IP,    --ip=IP         bind server only to specified IP, may appear\n"
				   "                            multiple times\n"
				   "                            (default: 0.0.0.0)\n";
		m_helpString = builder.str();
	}
	return m_helpString.c_str();
}

bool Setup::ParseSetupEntry( char const* name, char const* value )
{
	if ( strcmp( name, "LastChannel" ) == 0 ) m_lastChannel = atoi( value );
	else return false;
	return true;
}


bool Setup::CheckLibraryPath()
{
	ostringstream builder;
	builder << m_libraryPath << "/libtnt-live.so";
	if ( access( builder.str().c_str(), R_OK ) != 0 ) {
		esyslog( "ERROR: live can't open content library %s: %s", builder.str().c_str(), strerror( errno ) );
		cerr << "ERROR: live can't open content library " << builder << ": " << strerror( errno ) << endl;
		return false;
	}
	return true;
}

bool Setup::CheckServerPort()
{
	if ( m_serverPort <= 0 || m_serverPort > numeric_limits< uint16_t >::max() ) {
		esyslog( "ERROR: live server port %d is not a valid port number", m_serverPort );
		cerr << "ERROR: live server port " << m_serverPort << " is not a valid port number" << endl;
		return false;
	}
	return true;
}

bool Setup::CheckServerIps()
{
	if ( m_serverIps.empty() ) {
		m_serverIps.push_back( "0.0.0.0" );
		return true;
	}

	for ( IpList::const_iterator ip = m_serverIps.begin(); ip != m_serverIps.end(); ++ip ) {
		if ( inet_addr( ip->c_str() ) == static_cast< in_addr_t >( -1 ) ) {
			esyslog( "ERROR: live server ip %s is not a valid ip address", ip->c_str() );
			cerr << "ERROR: live server ip " << *ip << " is not a valid ip address" << endl;
			return false;
		}
	}
	return true;
}

} // namespace vdrlive
