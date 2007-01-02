#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vdr/plugin.h>
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
	builder << cPlugin::ConfigDirectory( PLUGIN_NAME_I18N ) << "/httpd.config";
	m_configPath = builder.str();
	
	ofstream file( m_configPath.c_str(), ios::out | ios::trunc );
	if ( !file ) {
		ostringstream builder;
		builder << "Can't open " << m_configPath << " for writing: " << strerror( errno );
		throw runtime_error( builder.str() );
	}

	// XXX modularize
	file << "MapUrl /([^.]+)(\\..+)? $1@libtnt-live" << std::endl;
	file << "Listen 0.0.0.0 8001" << std::endl;
	file << "PropertyFile " << m_propertiesPath << std::endl;
	file << "CompPath " << Setup::Get().GetLibraryPath() << "/" << std::endl;
}

void TntConfig::WriteProperties()
{
	ostringstream builder;
	builder << cPlugin::ConfigDirectory( PLUGIN_NAME_I18N ) << "/httpd.properties";
	m_propertiesPath = builder.str();
	
	ofstream file( m_propertiesPath.c_str(), ios::out | ios::trunc );
	if ( !file ) {
		ostringstream builder;
		builder << "Can't open " << m_propertiesPath << " for writing: " << strerror( errno );
		throw runtime_error( builder.str() );
	}

	// XXX modularize
	file << "rootLogger=INFO" << std::endl;
	file << "logger.tntnet=INFO" << std::endl;
}

TntConfig const& TntConfig::Get()
{
	static TntConfig instance;
	return instance;
}

} // namespace vdrlive
