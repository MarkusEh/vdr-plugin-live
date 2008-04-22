#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vdr/config.h>
#include <vdr/plugin.h>
#include "i18n.h"
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

	string const configDir(Plugin::GetConfigDirectory());

	ostringstream builder;
	builder << configDir << "/httpd.config";
	m_configPath = builder.str();

	ofstream file( m_configPath.c_str(), ios::out | ios::trunc );
	if ( !file ) {
		ostringstream builder;
		builder << "Can't open " << m_configPath << " for writing: " << strerror( errno );
		throw runtime_error( builder.str() );
	}

	// +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++
	// ------------------------------------------------------------------------
	// These MapUrl statements are very security sensitive!
	// A wrong mapping to content@ may allow retrieval of arbitrary files
	// from your VDR system via live.
	// Two meassures are taken against this in our implementation:
	// 1. The MapUrls need to be checked regulary against possible exploits
	//    One tool to do this can be found here:
	//      http://www.lumadis.be/regex/test_regex.php
	//    Newly inserted MapUrls should be marked with author and confirmed
	//    by a second party. (use source code comments for this)
	// 2. content.ecpp checks the given path to be
	//    a.  an absolute path starting at /
	//    b.  not containing ../ paths components
    //    In order to do so, the MapUrl statements must create absolute
	//    path arguments to content@
	// ------------------------------------------------------------------------
	// +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++


	file << "MapUrl ^/$ login@" << endl;

	// the following redirects vdr_request URL to the component
	// specified by the action parameter.
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/vdr_request/([^.]+) $1@" << endl;

	// the following selects the theme specific 'theme.css' file
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/themes/([^/]*)/css.*/(.+\\.css) content@ " << configDir << "/themes/$1/css/$2 text/css" << endl;

	// the following rules provide a search scheme for images. The first
	// rule where a image is found, terminates the search.
	// 1. /themes/<theme>/img/<imgname>.<ext>
	// 2. /img/<imgname>.<ext>
	// deprecated: 3. <imgname>.<ext> (builtin images)
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << configDir << "/themes/$1/img/$2.$3 image/$3" << endl;
	file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << configDir << "/img/$2.$3 image/$3" << endl;
	// deprecated: file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) $2@" << endl;

	// Epg images
	string const epgImgPath(LiveSetup().GetEpgImageDir());
	if (!epgImgPath.empty()) {
		// inserted by 'winni' -- EXPLOITABLE! (checked by tadi)
		// file << "MapUrl ^/epgimages/(.*)\\.(.+) content@ " << epgImgPath << "/$1.$2 image/$2" << endl;

		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		file << "MapUrl ^/epgimages/([^/]*)\\.([^./]+) content@ " << epgImgPath << "/$1.$2 image/$2" << endl;
	}

	// select additional (not build in) javascript.
	// WARNING: no path components with '.' in the name are allowed. Only
	// the basename may contain dots and must end with '.js'
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/js(/[^.]*)([^/]*\\.js) content@ " << configDir << "/js$1$2 text/javascript" << endl;

	// map to 'css/basename(uri)'
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/css.*/(.+) content@ " << configDir << "/css/$1 text/css" << endl;

	// map to 'img/basename(uri)'
	// inserted by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/img.*/(.+)\\.([^.]+) content@ " << configDir << "/img/$1.$2 image/$2" << endl;

	// Map favicon.ico into img directory
	file << "MapUrl ^/favicon.ico$ content@ " << configDir << "/img/favicon.ico image/x-icon" << endl;

	// insecure by default: DO NOT UNKOMMENT!!!
	// file << "MapUrl /([^/]+/.+) content@ $1" << endl;

	// takes first path components without 'extension' when it does not
	// contain '.'
	// modified by 'tadi' -- verified with above, but not counterchecked yet!
	file << "MapUrl ^/([^./]+)(.*)? $1@" << endl;

	file << "PropertyFile " << m_propertiesPath << endl;
	file << "SessionTimeout 86400" << endl;
	file << "DefaultContentType \"text/html; charset=" << LiveI18n().CharacterEncoding() << "\"" << endl;

	Setup::IpList const& ips = LiveSetup().GetServerIps();
	int port = LiveSetup().GetServerPort();
	for ( Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip ) {
		file << "Listen " << *ip << " " << port << endl;
	}

#ifdef TNTVERS7
	int s_port = LiveSetup().GetServerSslPort();
	string s_cert = LiveSetup().GetServerSslCert();

	if (s_cert.empty()) {
		s_cert = configDir + "/live.pem";
	}

	if ( ifstream( s_cert.c_str() ) ) {
		for ( Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip ) {
			file << "SslListen " << *ip << " " << s_port << " " << s_cert << endl;
		}
	}
	else {
		esyslog( "ERROR: %s: %s", s_cert.c_str(), strerror( errno ) );
	}
#endif
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
	file << "rootLogger=" << LiveSetup().GetTntnetLogLevel() << endl;
	file << "logger.tntnet=" << LiveSetup().GetTntnetLogLevel() << endl;
}

TntConfig const& TntConfig::Get()
{
	static TntConfig instance;
	return instance;
}

} // namespace vdrlive
