#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cxxtools/loginit.h>
#include <tnt/sessionscope.h>
#include <tnt/httpreply.h>
#include <vdr/config.h>
#include <vdr/plugin.h>
#include <vdr/videodir.h>
#include "i18n.h"
#include "live.h"
#include "setup.h"
#include "tntconfig.h"

namespace vdrlive {

	using namespace std;

	TntConfig::TntConfig()
	{
#if ! TNT_CONFIG_INTERNAL
		WriteConfig();
#endif
	}

#if ! TNT_CONFIG_INTERNAL
	void TntConfig::WriteConfig()
	{
		WriteProperties();

		string const configDir(Plugin::GetConfigDirectory());
#if APIVERSNUM > 10729
		string const resourceDir(Plugin::GetResourceDirectory());
#endif

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
#if APIVERSNUM > 10729
		file << "MapUrl ^/themes/([^/]*)/css.*/(.+\\.css) content@ " << resourceDir << "/themes/$1/css/$2 text/css" << endl;
#else
		file << "MapUrl ^/themes/([^/]*)/css.*/(.+\\.css) content@ " << configDir << "/themes/$1/css/$2 text/css" << endl;
#endif

		// the following rules provide a search scheme for images. The first
		// rule where a image is found, terminates the search.
		// 1. /themes/<theme>/img/<imgname>.<ext>
		// 2. /img/<imgname>.<ext>
		// deprecated: 3. <imgname>.<ext> (builtin images)
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
#if APIVERSNUM > 10729
		file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << resourceDir << "/themes/$1/img/$2.$3 image/$3" << endl;
		file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << resourceDir << "/img/$2.$3 image/$3" << endl;
#else
		file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << configDir << "/themes/$1/img/$2.$3 image/$3" << endl;
		file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) content@ " << configDir << "/img/$2.$3 image/$3" << endl;
#endif
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
#if APIVERSNUM > 10729
		file << "MapUrl ^/js(/[^.]*)([^/]*\\.js) content@ " << resourceDir << "/js$1$2 text/javascript" << endl;
#else
		file << "MapUrl ^/js(/[^.]*)([^/]*\\.js) content@ " << configDir << "/js$1$2 text/javascript" << endl;
#endif

		// map to 'css/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
#if APIVERSNUM > 10729
		file << "MapUrl ^/css.*/(.+) content@ " << resourceDir << "/css/$1 text/css" << endl;
#else
		file << "MapUrl ^/css.*/(.+) content@ " << configDir << "/css/$1 text/css" << endl;
#endif

		// map to 'img/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
#if APIVERSNUM > 10729
		file << "MapUrl ^/img.*/(.+)\\.([^.]+) content@ " << resourceDir << "/img/$1.$2 image/$2" << endl;
#else
		file << "MapUrl ^/img.*/(.+)\\.([^.]+) content@ " << configDir << "/img/$1.$2 image/$2" << endl;
#endif

		// Map favicon.ico into img directory
#if APIVERSNUM > 10729
		file << "MapUrl ^/favicon.ico$ content@ " << resourceDir << "/img/favicon.ico image/x-icon" << endl;
#else
		file << "MapUrl ^/favicon.ico$ content@ " << configDir << "/img/favicon.ico image/x-icon" << endl;
#endif

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
	}
#endif

#if ! TNT_CONFIG_INTERNAL
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
		file << "logger.cxxtools=" << LiveSetup().GetTntnetLogLevel() << endl;
	}
#endif

#if TNT_CONFIG_INTERNAL
	void TntConfig::Configure(tnt::Tntnet& app) const
	{
		string const configDir(Plugin::GetConfigDirectory());
#if APIVERSNUM > 10729
		string const resourceDir(Plugin::GetResourceDirectory());
#endif

		std::istringstream logConf(
			"rootLogger=" + LiveSetup().GetTntnetLogLevel() + "\n"
			"logger.tntnet=" + LiveSetup().GetTntnetLogLevel() + "\n"
			"logger.cxxtools=" + LiveSetup().GetTntnetLogLevel() + "\n"
			);
		log_init(logConf);

		// +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++ CAUTION +++
		// ------------------------------------------------------------------------
		// These mapUrl statements are very security sensitive!
		// A wrong mapping to content may allow retrieval of arbitrary files
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

		app.mapUrl("^/$", "login");

		// the following redirects vdr_request URL to the component
		// specified by the action parameter.
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/vdr_request/([^.]+)", "$1");

		// the following redirects play_video URL to the content component.
		// inserted by 'tadi' -- not verified, not counterchecked yet!
		//app.mapUrl("^/vlc/(.+)", "static@tntnet")
		//	.setPathInfo("/$1")
		//	.pushArg(string("DocumentRoot=") + VideoDirectory);

		// the following selects the theme specific 'theme.css' file
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/themes/([^/]*)/css.*/(.+\\.css)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/themes/$1/css/$2")
#else
			.setPathInfo(configDir + "/themes/$1/css/$2")
#endif
			.pushArg("text/css");

		// the following rules provide a search scheme for images. The first
		// rule where a image is found, terminates the search.
		// 1. /themes/<theme>/img/<imgname>.<ext>
		// 2. /img/<imgname>.<ext>
		// deprecated: 3. <imgname>.<ext> (builtin images)
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/themes/([^/]*)/img.*/(.+)\\.(.+)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/themes/$1/img/$2.$3")
#else
			.setPathInfo(configDir + "/themes/$1/img/$2.$3")
#endif
			.pushArg("image/$3");
		app.mapUrl("^/themes/([^/]*)/img.*/(.+)\\.(.+)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/img/$2.$3")
#else
			.setPathInfo(configDir + "/img/$2.$3")
#endif
			.pushArg("image/$3");
		// deprecated: file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) $2@" << endl;

		// Epg images
		string const epgImgPath(LiveSetup().GetEpgImageDir());
		if (!epgImgPath.empty()) {
			// inserted by 'tadi' -- verified with above, but not counterchecked yet!
			app.mapUrl("^/epgimages/([^/]*)\\.([^./]+)", "content")
				.setPathInfo(epgImgPath + "/$1.$2")
				.pushArg("image/$2");
		}

		// select additional (not build in) javascript.
		// WARNING: no path components with '.' in the name are allowed. Only
		// the basename may contain dots and must end with '.js'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/js(/[^.]*)([^/]*\\.js)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/js$1$2")
#else
			.setPathInfo(configDir + "/js$1$2")
#endif
			.pushArg("text/javascript");

		// map to 'css/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/css.*/(.+)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/css/$1")
#else
			.setPathInfo(configDir + "/css/$1")
#endif
			.pushArg("text/css");

		// map to 'img/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/img.*/(.+)\\.([^.]+)", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/img/$1.$2")
#else
			.setPathInfo(configDir + "/img/$1.$2")
#endif
			.pushArg("image/$2");

		// Map favicon.ico into img directory
		app.mapUrl("^/favicon.ico$", "content")
#if APIVERSNUM > 10729
			.setPathInfo(resourceDir + "/img/favicon.ico")
#else
			.setPathInfo(configDir + "/img/favicon.ico")
#endif
			.pushArg("image/x-icon");

		// takes first path components without 'extension' when it does not
		// contain '.'
		// modified by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/([^./]+)(.*)?", "$1");

		tnt::Sessionscope::setDefaultTimeout(86400);
		tnt::HttpReply::setDefaultContentType(string("text/html; charset=") + LiveI18n().CharacterEncoding());

		Setup::IpList const& ips = LiveSetup().GetServerIps();
		int port = LiveSetup().GetServerPort();
		size_t listenFailures = 0;
		for (Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip) {
			try {
				esyslog("[live] INFO: attempt to listen on ip = '%s'", ip->c_str());
				app.listen(*ip, port);
			}
			catch (exception const & ex) {
				esyslog("[live] ERROR: ip = %s is invalid: exception = %s", ip->c_str(), ex.what());
				if (++listenFailures == ips.size()) {
					// if no listener was initialized we throw at
					// least the last exception to the next layer.
					throw;
				}
			}
		}

#if TNT_SSL_SUPPORT
		int s_port = LiveSetup().GetServerSslPort();
		string s_cert = LiveSetup().GetServerSslCert();
		string s_key = LiveSetup().GetServerSslKey();

		if (s_cert.empty()) {
			s_cert = configDir + "/live.pem";
		}

		if (s_key.empty()) {
			s_key = configDir + "/live-key.pem";
		}

		if ( ifstream( s_cert.c_str() ) && ifstream( s_key.c_str() ) ) {
			for ( Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip ) {
				app.sslListen(s_cert, s_key, *ip, s_port);
			}
		}
		else {
			esyslog( "[live] ERROR: Unable to load cert/key (%s/%s): %s", s_cert.c_str(), s_key.c_str(), strerror( errno ) );
		}
#endif // TNT_SSL_SUPPORT
	}
#endif

	TntConfig const& TntConfig::Get()
	{
		static TntConfig instance;
		return instance;
	}
} // namespace vdrlive
