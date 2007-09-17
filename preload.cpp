#include <string>

#include <vdr/tools.h>

#include "filecache.h"

#include "preload.h"

using namespace std;

namespace vdrlive {

// to get an updated list of these files do:
// (cd live; find * -type f ! -wholename '*CVS*' ! -wholename '*themes*' ! -name '*~' ! -name '.*') | awk '{print "\"" $1 "\","}'}
// and clean out unneeded entries.

	void PreLoadFileCache(string const& configDir)
	{
		static char const * const preloadFiles[] = {
			"css/siteprefs.css",
			"img/rounded-box-blue-bl.png",
			"img/rounded-box-blue-br.png",
			"img/rounded-box-blue-ml.png",
			"img/rounded-box-blue-mr.png",
			"img/rounded-box-blue-tr.png",
			"img/rounded-box-green-bl.png",
			"img/rounded-box-blue-tl.png",
			"img/rounded-box-green-br.png",
			"img/rounded-box-green-ml.png",
			"img/rounded-box-green-mr.png",
			"img/del.png",
			"img/info-win-t-r.png",
			"img/info-win-m-l.png",
			"img/info-win-m-r.png",
			"img/info-win-b-l.png",
			"img/info-win-b-r.png",
			"img/close_red.png",
			"img/info-win-t-l.png",
			"img/rounded-box-green-tl.png",
			"img/rounded-box-green-tr.png",
			"img/rounded-box-orange-bl.png",
			"img/rounded-box-orange-br.png",
			"img/rounded-box-orange-ml.png",
			"img/rounded-box-orange-mr.png",
			"img/rounded-box-orange-tl.png",
			"img/rounded-box-orange-tr.png",
			"img/active.png",
			"img/arrow.png",
			"img/bg_box_h.png",
			"img/bg_box_l.png",
			"img/bg_box_r.png",
			"img/bg_header_h.png",
			"img/bg_header_l.png",
			"img/bg_header_r.png",
			"img/bg_line.png",
			"img/bg_line_top.png",
			"img/bg_tools.png",
			"img/button_blue.png",
			"img/button_green.png",
			"img/button_new.png",
			"img/button_red.png",
			"img/button_yellow.png",
			"img/close.png",
			"img/edit.png",
			"img/ffw.png",
			"img/file.png",
			"img/folder_closed.png",
			"img/folder_open.png",
			"img/help.png",
			"img/imdb.png",
			"img/inactive.png",
			"img/logo_login.png",
			"img/logo.png",
			"img/menu_line_bg.png",
			"img/minus.png",
			"img/movie.png",
			"img/on_dvd.png",
			"img/one_downarrow.png",
			"img/one_uparrow.png",
			"img/pause.png",
			"img/play.png",
			"img/plus.png",
			"img/record.png",
			"img/record_timer.png",
			"img/reload.png",
			"img/rwd.png",
			"img/search.png",
			"img/stop.png",
			"img/stop_update.png",
			"img/transparent.png",
			"img/zap.png",
			"img/remotecontrol.jpg",
			"img/tv.jpg",
			"img/arrow_rec.gif",
			"img/favicon.ico",
			"js/live/vdr_status.js",
			"js/live/infowin.js",
			"js/live/liveajax.js",
			"js/live/hinttips.js",
			"js/live/pageenhance.js",
			"js/mootools/mootools.v1.11.js",
			0
		};

		FileCache& fc = LiveFileCache();
		size_t i = 0;
		while (preloadFiles[i]) {
			FileCache::ptr_type f = fc.get(configDir + "/" + preloadFiles[i]);
			if (0 == f.get()) {
				isyslog("LIVE: can't preload %s/%s! Generated pages might be degraded!", configDir.c_str(), preloadFiles[i]);
			}
			i++;
		}
		isyslog("LIVE: initial file cache has %d entries and needs %d bytes of data!", fc.count(), fc.weight());
	}
}
