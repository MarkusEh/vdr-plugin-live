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
	file << "MapUrl ^/$ login@" << endl;
	file << "MapUrl ^/themes/([^/]*)/css.*/(.+\\.css) content@ themes/$1/css/$2 text/css" << endl;

	// the following rules provide a search scheme for images. The first
	// rule where a image is found, terminates the search.
	// 1. /themes/<theme>/img/<imgname>.<ext>
	// 2. /dist/img/<imgname>.<ext>
	// 3. <imgname>.<ext> (builtin images)
	file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ themes/$1/img/$2.$3 image/$3" << endl;
	file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ common/img/$2.$3 image/$3" << endl;
	file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) $2@" << endl;

	file << "MapUrl ^/css.*/(.+) content@ css/$1 text/css" << endl;
	file << "MapUrl /([^/]+/.+) content@ $1" << endl;
	file << "MapUrl /([^.]+)(\\..+)? $1@" << endl;
	file << "PropertyFile " << m_propertiesPath << endl;
	file << "SessionTimeout 86400" << endl;

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
