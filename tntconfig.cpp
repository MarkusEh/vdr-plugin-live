#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vdr/config.h>
#include <vdr/plugin.h>
#include "live.h"
#include "setup.h"
#include "tntconfig.h"

namespace vdrlive {

using namespace std;

TntConfig::TntConfig()
{
	WriteConfig();
}

void TntConfig::WriteConfig()
{
	WriteProperties();

	ostringstream builder;
	builder << Plugin::GetConfigDirectory() << "/httpd.config";
	m_configPath = builder.str();
	
	ofstream file( m_configPath.c_str(), ios::out | ios::trunc );
	if ( !file ) {
		ostringstream builder;
		builder << "Can't open " << m_configPath << " for writing: " << strerror( errno );
		throw runtime_error( builder.str() );
	}

	// XXX modularize
	file << "MapUrl ^/$ whats_on_now@" << endl;
	file << "MapUrl /([^.]+)(\\..+)? $1@" << endl;
	file << "PropertyFile " << m_propertiesPath << endl;

	Setup::IpList const& ips = LiveSetup().GetServerIps();
	int port = LiveSetup().GetServerPort();
	for ( Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip ) {
		file << "Listen " << *ip << " " << port << endl;
	}
}

void TntConfig::WriteProperties()
{
	ostringstream builder;
	builder << Plugin::GetConfigDirectory() << "/httpd.properties";
	m_propertiesPath = builder.str();
	
	ofstream file( m_propertiesPath.c_str(), ios::out | ios::trunc );
	if ( !file ) {
		ostringstream builder;
		builder << "Can't open " << m_propertiesPath << " for writing: " << strerror( errno );
		throw runtime_error( builder.str() );
	}

	// XXX modularize
	file << "rootLogger=INFO" << endl;
	file << "logger.tntnet=INFO" << endl;
}

TntConfig const& TntConfig::Get()
{
	static TntConfig instance;
	return instance;
}

} // namespace vdrlive
