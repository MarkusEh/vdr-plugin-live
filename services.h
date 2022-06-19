#ifndef __TVSCRAPER_SERVICES_H
#define __TVSCRAPER_SERVICES_H
#include <vdr/epg.h>
#include <vdr/recording.h>
#include <string>
#include <vector>

enum tvType {
    tSeries,
    tMovie,
    tNone,
};


/*********************************************************************
* Helper Structures
*********************************************************************/
class cTvMedia {
public:
  cTvMedia(void) {
	path = "";
	width = height = 0;
  };
  std::string path;
  int width;
  int height;
};

class cActor {
public:
    cActor(void) {
        name = "";
        role = "";
    };
    std::string name;
    std::string role;
    cTvMedia actorThumb;
};


class cEpisode {
public:
    cEpisode(void) {
        number = 0;
        season = 0;
        name = "";
        firstAired = "";
        guestStars = "";
        overview = "";
        rating = 0.0;
    };
    int number;
    int season;
    std::string name;
    std::string firstAired;
    std::string guestStars;
    std::string overview;
    float rating;
    cTvMedia episodeImage;
};

class cEpisode2 {
public:
    int number;
    int season;
    int absoluteNumber;
    std::string name;
    std::string firstAired;
    std::vector<cActor> guestStars;
    std::string overview;
    float vote_average;
    int vote_count;
    cTvMedia episodeImage;
    std::string episodeImageUrl;
    std::vector<std::string> director;
    std::vector<std::string> writer;
    std::string IMDB_ID;
};

/*********************************************************************
* Data Structures for Service Calls
*********************************************************************/

// Data structure for service "GetEventType"
class ScraperGetEventType {
public:
	ScraperGetEventType(void) {
		event = NULL;
        recording = NULL;
		type = tNone;
		movieId = 0;
		seriesId = 0;
		episodeId = 0;
	};
// in
    const cEvent *event;             // check type for this event 
    const cRecording *recording;     // or for this recording
//out
    tvType type;                	 //typeSeries or typeMovie
    int movieId;
    int seriesId;
    int episodeId;
};

//Data structure for full series and episode information
class cMovie {
public:
    cMovie(void) {
        title = "";
        originalTitle = "";
        tagline = "";    
        overview = "";
        adult = false;
        collectionName = "";
        budget = 0;
        revenue = 0;
        genres = "";
        homepage = "";
        releaseDate = "";
        runtime = 0;
        popularity = 0.0;
        voteAverage = 0.0;
    };
//IN
    int movieId;                    // movieId fetched from ScraperGetEventType
//OUT    
    std::string title;
    std::string originalTitle;
    std::string tagline;    
    std::string overview;
    bool adult;
    std::string collectionName;
    int budget;
    int revenue;
    std::string genres;
    std::string homepage;
    std::string releaseDate;
    int runtime;
    float popularity;
    float voteAverage;
    cTvMedia poster;
    cTvMedia fanart;
    cTvMedia collectionPoster;
    cTvMedia collectionFanart;
    std::vector<cActor> actors;
};

//Data structure for full series and episode information
class cSeries {
public:
    cSeries(void) {
        seriesId = 0;
        episodeId = 0;
        name = "";
        overview = "";
        firstAired = "";
        network = "";
        genre = "";
        rating = 0.0;
        status = "";
    };
//IN
    int seriesId;                   // seriesId fetched from ScraperGetEventType
    int episodeId;                  // episodeId fetched from ScraperGetEventType
//OUT
    std::string name;
    std::string overview;
    std::string firstAired;
    std::string network;
    std::string genre;
    float rating;
    std::string status;
    cEpisode episode;
    std::vector<cActor> actors;
    std::vector<cTvMedia> posters;
    std::vector<cTvMedia> banners;
    std::vector<cTvMedia> fanarts;
    cTvMedia seasonPoster;
};

// Data structure for service "GetPosterBanner"
class ScraperGetPosterBanner {
public:
	ScraperGetPosterBanner(void) {
		type = tNone;
	};
// in
    const cEvent *event;             // check type for this event 
//out
    tvType type;                	 //typeSeries or typeMovie
    cTvMedia poster;
    cTvMedia banner;
};

// Data structure for service "GetPoster"
class ScraperGetPoster {
public:
// in
    const cEvent *event;             // check type for this event
    const cRecording *recording;     // or for this recording
//out
    cTvMedia poster;
};

// Data structure for service "GetPosterThumb"
class ScraperGetPosterThumb {
public:
// in
    const cEvent *event;             // check type for this event
    const cRecording *recording;     // or for this recording
//out
    cTvMedia poster;
};
// Data structure for Kodi
// see https://alwinesch.github.io/group__cpp__kodi__addon__pvr___defs__epg___p_v_r_e_p_g_tag.html
// see https://alwinesch.github.io/group__cpp__kodi__addon__pvr___defs___recording___p_v_r_recording.html
// Series link: If set for an epg-based timer rule, matching events will be found by checking strSeriesLink instead of strTitle (and bFullTextEpgSearch)  see https://github.com/xbmc/xbmc/pull/12609/files
// tag.SetFlags(PVR_RECORDING_FLAG_IS_SERIES);
// SetSeriesNumber (this is the season number)

//Data structure for full information
class cScraperMovieOrTv {
public:
//IN
    const cEvent *event;             // provide data forthis event 
    const cRecording *recording;     // or for this recording
    bool httpImagePaths;             // if true, provide http paths to images
    bool media;                      // if true, provide local filenames for media
//OUT
    bool found;
    bool movie;
    std::string title;
    std::string originalTitle;
    std::string tagline;
    std::string overview;
    std::vector<std::string> genres;
    std::string homepage;
    std::string releaseDate;  // for TV shows: firstAired
    bool adult;
    std::vector<int> runtimes;
    float popularity;
    float voteAverage;
    int voteCount;
    std::vector<std::string> productionCountries;
    std::vector<cActor> actors;
    std::string IMDB_ID;
    std::string posterUrl;   // only if httpImagePaths == true
    std::string fanartUrl;   // only if httpImagePaths == true
    std::vector<cTvMedia> posters;
    std::vector<cTvMedia> banners;
    std::vector<cTvMedia> fanarts;
// only for movies
    int budget;
    int revenue;
    int collectionId;
    std::string collectionName;
    cTvMedia collectionPoster;
    cTvMedia collectionFanart;
    std::vector<std::string> director;
    std::vector<std::string> writer;
// only for TV Shows
    std::string status;
    std::vector<std::string> networks;
    std::vector<std::string> createdBy;
// episode related
    bool episodeFound;
    cTvMedia seasonPoster;
    cEpisode2 episode;
};


#endif // __TVSCRAPER_SERVICES_H
