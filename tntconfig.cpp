#include "tntconfig.h"

#include "i18n.h"
#include "live.h"
#include "setup.h"

#if TNT_LOG_SERINFO
#include <cxxtools/log.h>
#include <cxxtools/xml/xmldeserializer.h>
#else
#include <cxxtools/loginit.h>
#endif

#if TNTVERSION >= 31000
#include <cxxtools/sslctx.h>
#endif

#include "services.h"

namespace vdrlive {

  TntConfig::TntConfig()
  {
  }

  namespace {
    std::string GetResourcePath()
    {
      std::string resourceDir(Plugin::GetResourceDirectory());
      return resourceDir;
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

  void TntConfig::ConfigureTvscraper(tnt::Tntnet& app, const std::string &tvscraperImageDir) const
        {
// make the Tvscraper images available in the web server
    // Images from Tvscraper TheMovieDb: Movies
      MapUrl(app,
           "^/tvscraper/movies/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies",
           "/$1.$2",
           "image/$2");
    // Images from Tvscraper TheMovieDb: Collections
      MapUrl(app,
           "^/tvscraper/movies/collections/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies/collections",
           "/$1.$2",
           "image/$2");
    // Images from Tvscraper TheMovieDb: Actors
      MapUrl(app,
           "^/tvscraper/movies/actors/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies/actors",
           "/$1.$2",
           "image/$2");
    // Images from Tvscraper TheMovieDb: TV shows
      MapUrl(app,
           "^/tvscraper/movies/tv/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies/tv",
           "/$1/$2.$3",
           "image/$3");
    // Images from Tvscraper TheMovieDb: TV shows, season
      MapUrl(app,
           "^/tvscraper/movies/tv/([^/]*)/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies/tv",
           "/$1/$2/$3.$4",
           "image/$4");
    // Images from Tvscraper TheMovieDb: TV shows, episode
      MapUrl(app,
           "^/tvscraper/movies/tv/([^/]*)/([^/]*)/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "movies/tv",
           "/$1/$2/$3/$4.$5",
           "image/$5");
    // Images from Tvscraper TheTvDb: TV shows
      MapUrl(app,
           "^/tvscraper/series/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           tvscraperImageDir + "series",
           "/$1/$2.$3",
           "image/$3");
    // Images from Tvscraper TheTvDb: TV shows, episode images
      MapUrl(app,
           "^/tvscraper/series/([^/]*)/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           LiveSetup().GetTvscraperImageDir() + "series",
           "/$1/$2/$3.$4",
           "image/$4");
    // Images from Tvscraper, from external EPG provider (start time, channel, image)
      MapUrl(app,
           "^/tvscraper/epg/([^/]*)/([^/]*)/([^/]*)\\.([^./]+)",
           "content",
           LiveSetup().GetTvscraperImageDir() + "epg",
           "/$1/$2/$3.$4",
           "image/$4");
    // Images from Tvscraper, from external EPG provider, in recordings (start time, channel, image)
      MapUrl(app,
           "^/tvscraper/recordings/([^/]*)\\.([^./]+)",
           "content",
           LiveSetup().GetTvscraperImageDir() + "recordings",
           "/$1.$2",
           "image/$2");
        }
  void TntConfig::Configure(tnt::Tntnet& app) const
  {
    std::string const configDir(Plugin::GetConfigDirectory());

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
    // Two measures are taken against this in our implementation:
    // 1. The MapUrls need to be checked regularly against possible exploits
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
    //  .setPathInfo("/$1")
    //  .pushArg(string("DocumentRoot=") + VideoDirectory);

    // the following selects the theme specific 'theme.css' file
    // inserted by 'tadi' -- verified with above, but not counterchecked yet!
    MapUrl(app,
         "^/themes/([^/]*)/css.*/(.+\\.css)",
         "content",
         GetResourcePath(),
         "/themes/$1/css/$2",
         "text/css");

    // the following rules enable SVG file support, which require the special
    // content type "image/svg+xml" for inline display in browsers
    // inserted by 'flell' -- verified with above, but not counterchecked yet!
    MapUrl(app,
         "^/themes/([^/]*)/img.*/(.+)\\.svg",
         "content",
         GetResourcePath(),
         "/themes/$1/img/$2.svg",
         "image/svg+xml");

    MapUrl(app,
         "^/themes/([^/]*)/img.*/(.+)\\.svg",
         "content",
         GetResourcePath(),
         "/img/$2.svg",
         "image/svg+xml");

    MapUrl(app,
         "^/img.*/(.+)\\.svg",
         "content",
         GetResourcePath(),
         "/img/$1.svg",
         "image/svg+xml");

    // the following rule enables channel logo support
    // inserted by 'flell' -- verified with above, but not counterchecked yet!
    if (!LiveSetup().GetChanLogoDir().empty() ) {
          MapUrl(app,
               "^/chanlogos/(.+)\\.png",
               "content",
               LiveSetup().GetChanLogoDir(),
               "/$1.png",
           "image/png");
    }

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

// get image dir from plugin Tvscraper
    static cPlugin *pScraper = LiveSetup().GetPluginScraper();
    if (pScraper) {
// plugin Tvscraper or scraper2vdr is available
// first try cEnvironment, which is also available in scraper2vdr
      cEnvironment environment;
      if (pScraper->Service("GetEnvironment", &environment) ) { // available in tvscraper 1.2.1 and later
// plugin Tvscraper/scraper2vdr supports the service interface cEnvironment
        esyslog("live: INFO: set image dir from GetEnvironment to '%s'", environment.basePath.c_str());
        LiveSetup().SetTvscraperImageDir(environment.basePath);
      } else {
        cGetScraperImageDir getScraperImageDir;
        if (getScraperImageDir.call(pScraper) ) {
// plugin Tvscraper supports the service interface GetScraperImageDir (version 1.05 or newer)
          esyslog("live: WARNING: set image dir from deprecated GetScraperImageDir to '%s'", getScraperImageDir.scraperImageDir.c_str());
          LiveSetup().SetTvscraperImageDir(getScraperImageDir.scraperImageDir);
        }
      }
    }
    if (!LiveSetup().GetTvscraperImageDir().empty()) {
      ConfigureTvscraper(app, LiveSetup().GetTvscraperImageDir());
    }

    // EPG images
    std::string const epgImgPath(LiveSetup().GetEpgImageDir());
    if (!epgImgPath.empty()) {
      // inserted by 'tadi' -- verified with above, but not counterchecked yet!
      MapUrl(app,
           "^/epgimages/([^/]*)\\.([^./]+)",
           "content",
           epgImgPath,
           "/$1.$2",
           "image/$2");
    }

    // recording images
    MapUrl(app,
         "^/recimages/([^/]*)/([^/]*)\\.([^./]+)",
         "content",
         "",
         tmpImageDir "$1_$2.$3",
         "image/$3");

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

    // map to 'html/basename(uri)'
    // inserted by 'MarkusE' -- verified with above, but not counterchecked yet!
    MapUrl(app,
         "^/html.*/(.+)",
         "content",
         GetResourcePath(),
         "/html/$1",
         "text/html");

    // Map favicon.ico into img directory
    MapUrl(app,
         "^/favicon.ico$",
         "content",
         GetResourcePath(),
         "/img/favicon.ico",
         "image/x-icon");

    // Map HLS streaming data folder. Module stream_data.ecpp is used to serve content.
    app.mapUrl("^/media/(.+)", "stream_data");

    // takes first path components without 'extension' when it does not
    // contain '.'
    // modified by 'tadi' -- verified with above, but not counterchecked yet!
    app.mapUrl("^/([^./]+)(.*)?", "$1");

#if TNT_GLOBAL_TNTCONFIG
    tnt::TntConfig::it().sessionTimeout = 86400;
    tnt::TntConfig::it().defaultContentType = std::string("text/html; charset=") + LiveI18n().CharacterEncoding();
#else
    tnt::Sessionscope::setDefaultTimeout(86400);
    tnt::HttpReply::setDefaultContentType(std::string("text/html; charset=") + LiveI18n().CharacterEncoding());
#endif

    Setup::IpList const& ips = LiveSetup().GetServerIps();
    int port = LiveSetup().GetServerPort();
    size_t listenFailures = 0;
    for (Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip) {
      try {
        esyslog("live: INFO: attempt to listen on ip = '%s'", ip->c_str());
        app.listen(*ip, port);
      }
      catch (std::exception const & ex) {
        esyslog("live: ERROR: ip = %s is invalid: exception = %s", ip->c_str(), ex.what());
        if (++listenFailures == ips.size()) {
          // if no listener was initialized we throw at
          // least the last exception to the next layer.
          throw;
        }
      }
    }

    int s_port = LiveSetup().GetServerSslPort();
    if (s_port > 0) {
      std::string s_cert = LiveSetup().GetServerSslCert();
      std::string s_key = LiveSetup().GetServerSslKey();

      if (s_cert.empty()) {
        s_cert = configDir + "/live.pem";
      }

      if (s_key.empty()) {
        s_key = configDir + "/live-key.pem";
      }

      if (std::ifstream( s_cert.c_str() ) && std::ifstream( s_key.c_str() ) ) {
        for ( Setup::IpList::const_iterator ip = ips.begin(); ip != ips.end(); ++ip ) {
#if TNTVERSION < 31000
          app.sslListen(s_cert, s_key, *ip, s_port);
#else
          cxxtools::SslCtx sslCtx;
          sslCtx.loadCertificateFile(s_cert, s_key);
          app.listen(*ip, s_port, sslCtx);
#endif
        }
      }
      else {
        esyslog( "live: ERROR: Unable to load cert/key (%s / %s): %s", s_cert.c_str(), s_key.c_str(), strerror( errno ) );
      }
    }
    else {
      isyslog( "live: INFO: SSL port %d specified, no SSL Web server will be started", s_port);
    }
  }

  TntConfig const& TntConfig::Get()
  {
    static TntConfig instance;
    return instance;
  }
} // namespace vdrlive
