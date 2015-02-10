#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "tntfeatures.h"
#if TNT_LOG_SERINFO
#include <cxxtools/log.h>
#include <cxxtools/xml/xmldeserializer.h>
#else
#include <cxxtools/loginit.h>
#endif
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
	}

	namespace {
		std::string GetResourcePath()
		{
#if APIVERSNUM > 10729
			string resourceDir(Plugin::GetResourceDirectory());
			return resourceDir;
#else
			string configDir(Plugin::GetConfigDirectory());
			return configDir;
#endif
		}

		void MapUrl(tnt::Tntnet & app, const char *rule, const char * component, std::string const & instPath, const char * pathInfo, const char * mime_type)
		{
#if TNT_MAPURL_NAMED_ARGS
			tnt::Mapping::args_type argMap;
			argMap.insert(std::make_pair("mime-type", mime_type));
#endif
			app.mapUrl(rule, component)
				.setPathInfo(instPath + pathInfo)
#if TNT_MAPURL_NAMED_ARGS
				.setArgs(argMap);
#else
				.pushArg(mime_type);
#endif
		}
	}

	void TntConfig::Configure(tnt::Tntnet& app) const
	{
		string const configDir(Plugin::GetConfigDirectory());

#if TNT_LOG_SERINFO
		cxxtools::SerializationInfo si;
		std::istringstream logXmlConf(
			"<logging>\n"
			"  <rootlogger>" + LiveSetup().GetTntnetLogLevel() + "</rootlogger>\n"
			"  <loggers>\n"
			"    <logger>\n"
			"      <category>cxxtools</category>\n"
			"      <level>" + LiveSetup().GetTntnetLogLevel() + "</level>\n"
			"    </logger>\n"
			"    <logger>\n"
			"      <category>tntnet</category>\n"
			"      <level>" + LiveSetup().GetTntnetLogLevel() + "</level>\n"
			"    </logger>\n"
			"  </loggers>\n"
			"</logging>\n"
			);
		cxxtools::xml::XmlDeserializer d(logXmlConf);
		d.deserialize(si);
		log_init(si);
#else
		std::istringstream logConf(
			"rootLogger=" + LiveSetup().GetTntnetLogLevel() + "\n"
			"logger.tntnet=" + LiveSetup().GetTntnetLogLevel() + "\n"
			"logger.cxxtools=" + LiveSetup().GetTntnetLogLevel() + "\n"
			);

		log_init(logConf);
#endif

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
		MapUrl(app,
			   "^/themes/([^/]*)/css.*/(.+\\.css)",
			   "content",
			   GetResourcePath(),
			   "/themes/$1/css/$2",
			   "text/css");

		// the following rules provide a search scheme for images. The first
		// rule where a image is found, terminates the search.
		// 1. /themes/<theme>/img/<imgname>.<ext>
		// 2. /img/<imgname>.<ext>
		// deprecated: 3. <imgname>.<ext> (builtin images)
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		MapUrl(app,
			   "^/themes/([^/]*)/img.*/(.+)\\.(.+)",
			   "content",
			   GetResourcePath(),
			   "/themes/$1/img/$2.$3",
			   "image/$3");

		MapUrl(app,
			   "^/themes/([^/]*)/img.*/(.+)\\.(.+)",
			   "content",
			   GetResourcePath(),
			   "/img/$2.$3",
			   "image/$3");
		// deprecated: file << "MapUrl ^/themes/([^/]*)/img.*/(.+)\\.(.+) $2@" << endl;

		// Epg images
		string const epgImgPath(LiveSetup().GetEpgImageDir());
		if (!epgImgPath.empty()) {
			// inserted by 'tadi' -- verified with above, but not counterchecked yet!
			MapUrl(app,
				   "^/epgimages/([^/]*)\\.([^./]+)",
				   "content",
				   epgImgPath,
				   "/$1.$2",
				   "image/$2");
		}

		// select additional (not build in) javascript.
		// WARNING: no path components with '.' in the name are allowed. Only
		// the basename may contain dots and must end with '.js'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		MapUrl(app,
			   "^/js(/[^.]*)([^/]*\\.js)",
			   "content",
			   GetResourcePath(),
			   "/js$1$2",
			   "text/javascript");

		// map to 'css/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		MapUrl(app,
			   "^/css.*/(.+)",
			   "content",
			   GetResourcePath(),
			   "/css/$1",
			   "text/css");

		// map to 'img/basename(uri)'
		// inserted by 'tadi' -- verified with above, but not counterchecked yet!
		MapUrl(app,
			   "^/img.*/(.+)\\.([^.]+)",
			   "content",
			   GetResourcePath(),
			   "/img/$1.$2",
			   "image/$2");

		// Map favicon.ico into img directory
		MapUrl(app,
			   "^/favicon.ico$",
			   "content",
			   GetResourcePath(),
			   "/img/favicon.ico",
			   "image/x-icon");

		// takes first path components without 'extension' when it does not
		// contain '.'
		// modified by 'tadi' -- verified with above, but not counterchecked yet!
		app.mapUrl("^/([^./]+)(.*)?", "$1");

#if TNT_GLOBAL_TNTCONFIG
		tnt::TntConfig::it().sessionTimeout = 86400;
		tnt::TntConfig::it().defaultContentType = string("text/html; charset=") + LiveI18n().CharacterEncoding();
#else
		tnt::Sessionscope::setDefaultTimeout(86400);
		tnt::HttpReply::setDefaultContentType(string("text/html; charset=") + LiveI18n().CharacterEncoding());
#endif

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
	}

	TntConfig const& TntConfig::Get()
	{
		static TntConfig instance;
		return instance;
	}
} // namespace vdrlive
