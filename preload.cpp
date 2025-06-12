
#include "preload.h"

#include "filecache.h"

#include <vdr/tools.h>


namespace vdrlive {

// to get an updated list of these files do:
// (cd live; find * -type f ! -wholename '*CVS*' ! -wholename '*themes*' ! -name '*~' ! -name '.*') | awk '{print "\"" $1 "\","}'
// and clean out unneeded entries.

  void PreLoadFileCache(std::string const& configDir)
  {
    static char const * const preloadFiles[] = {
      "css/siteprefs.css",
      "css/styles.css",
      "html/back.html",
      "img/active.png",
      "img/arrow.png",
      "img/arrow_rec.png",
      "img/button_new.png",
      "img/close.png",
      "img/close_red.png",
      "img/del.png",
      "img/edit.png",
      "img/favicon.ico",
      "img/ffw.png",
      "img/file.png",
      "img/folder_closed.png",
      "img/folder_open.png",
      "img/hd.png",
      "img/help.png",
      "img/imdb.png",
      "img/inactive.png",
      "img/logo.png",
      "img/logo_login.png",
      "img/minus.png",
      "img/movie.png",
      "img/NoRecordingErrors.png",
      "img/NotCheckedForRecordingErrors.png",
      "img/on_dvd.png",
      "img/one_downarrow.png",
      "img/one_uparrow.png",
      "img/pause.png",
      "img/play.png",
      "img/playlist.png",
      "img/plus.png",
      "img/rd.png",
      "img/record.png",
      "img/record_timer_inactive.png",
      "img/record_timer_active.png",
      "img/RecordingErrors.png",
      "img/reload.png",
      "img/remote_control.png",
      "img/resize.png",
      "img/rwd.png",
      "img/sd.png",
      "img/search.png",
      "img/stop.png",
      "img/stop_update.png",
      "img/timer_conflict.png",
      "img/transparent.png",
      "img/tv.png",
      "img/zap.png",
      "js/live/browserwin.js",
      "js/live/createHtml.js",
      "js/live/epg_tt_box.js",
      "js/live/hinttips.js",
      "js/live/infowin.js",
      "js/live/liveajax.js",
      "js/live/pageenhance.js",
      "js/live/scrolling.js",
      "js/live/treeview.js",
      "js/live/vdr_status.js",
      "js/mootools/mootools.v1.11.js",
      0
    };

    FileCache& fc = LiveFileCache();
    size_t i = 0;
    while (preloadFiles[i]) {
      FileCache::ptr_type f = fc.get(configDir + "/" + preloadFiles[i]);
      if (0 == f.get()) {
        isyslog("live: can't preload %s/%s! Generated pages might be degraded!", configDir.c_str(), preloadFiles[i]);
      }
      i++;
    }
    isyslog("live: initial file cache has %zu entries and needs %zu bytes of data!", fc.count(), fc.weight());
  }
}
