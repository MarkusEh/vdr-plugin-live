#ifndef __LIVE_SERVICES_LIVE_H
#define __LIVE_SERVICES_LIVE_H

class cLiveImageProvider {
  public:
    virtual std::string getImageUrl(const std::string &imagePath, bool fullPath = true) = 0;
      ///< input: imagePath on file system.
      ///<   if fullPath = true: e.g. /var/cache/vdr/plugins/tvscraper/movies/300803_poster.jpg
      ///<   if fullPath = false: e.g.                                 movies/300803_poster.jpg
      ///<   for images returned by the "old" Tvscraper interface: use the default fullPath = true
      ///< output:
      ///<   in case of no error:
      ///<     URL to the image, e.g. http://rpi.fritz.box:8008/tvscraper/movies/300803_poster.jpg
      ///<   in case of errors:
      ///<     If fullPath = true, the input (imagePath) will be returned
      ///<     If fullPath = false, the full file system path will be returned (if possible)
      ///<     Also, an error message will be written to the system log
      ///< possible errors:
      ///<   missing server url. This url must be provided to live with -u URL,  --url=URL
      ///<   missing directory for scraper images:
      ///<     tvscraper and scraper2vdr make this directory available to live with the service interface
      ///<     tvscraper version 1.2.1 or later is required
      ///<   image path %s does not start with %s: Will only occur if fullPath == true
      ///<     imagePath does not start with directory for scraper images.
      ///<     example:
      ///<       directory for scraper images: /var/cache/vdr/plugins/tvscraper/
      ///<       imagePath = /tmp/test.img
      ///<       In this example, this error will occur.
      ///<       This is a restriction of live, implemented for security reasons:
      ///<       Only files under the scraper images path will be delivered
    virtual ~cLiveImageProvider() {}
};
// service to return cLiveImageProvider instance
class cGetLiveImageProvider {
public:
  cPlugin *call(cPlugin *pLive = NULL) {
    if (!pLive) return cPluginManager::CallFirstService("GetLiveImageProvider", this);
    return pLive->Service("GetLiveImageProvider", this)?pLive:NULL;
  }
//IN: Use constructor to set these values
// No input parameters
//OUT
   std::unique_ptr<cLiveImageProvider> m_liveImageProvider;
};

#endif // __LIVE_SERVICES_LIVE_H
