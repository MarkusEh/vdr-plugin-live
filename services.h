#ifndef __TVSCRAPER_SERVICES_H
#define __TVSCRAPER_SERVICES_H
#include <vdr/epg.h>
#include <vdr/recording.h>
#include <vdr/plugin.h>
#include <string>
#include <vector>

/*********************************************************************
* Helper Structures
*********************************************************************/
enum tvType {
    tSeries,
    tMovie,
    tNone,
};

enum class eVideoType {
  none = 0,
  movie = 1,
  tvShow = 2,
  musicVideo = 3,
};

enum class eOrientation {
  none = 0,
  banner = 1,
  landscape = 2,
  portrait = 3,
};

class cOrientations {
  public:
    cOrientations(eOrientation first = eOrientation::none, eOrientation second = eOrientation::none, eOrientation third = eOrientation::none) {
      m_orientations = (int)first | (int(second)<<3) | (int(third)<<6);
    }
  private:
    friend class cOrientationsInt;
    int m_orientations;
};
 

// for movies: movie (itself) or collection
// for TV Shows: episode, season, TV Show (itself), any season (fallback if on other image is available)
// example: image for event/recording: first = episodeMovie, second = seasonMovie, third = TV_ShowCollection, forth = anySeasonCollection
// if you don't want the episode still:
// first = seasonMovie, second = TV_ShowCollection, third = anySeasonCollection
// for a node with movies: TV_ShowCollection
// for a node representing a single season: first = seasonMovie, second = TV_ShowCollection, third = anySeasonCollection
enum class eImageLevel {
  none = 0,
  episodeMovie = 1,      // for TV Shows: episode. For movies: movie (itself)
  seasonMovie  = 2,      // for TV Shows: season.  For movies: movie (itself)
  tvShowCollection = 3, // for TV Shows: itself.  For movies: collection
  anySeasonCollection = 4,
};

class cImageLevels {
  public:
    cImageLevels(eImageLevel first = eImageLevel::none, eImageLevel second = eImageLevel::none, eImageLevel third = eImageLevel::none, eImageLevel forth = eImageLevel::none){
      m_imageLevels = (int)first | (int(second)<<3) | (int(third)<<6) | (int(forth)<<9);
    }
  private:
    friend class cImageLevelsInt;
    int m_imageLevels;
};
 

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

// Data structure for service "GetScraperImageDir"
class cGetScraperImageDir {
public:
//in: nothing, no input required
//out
  std::string scraperImageDir;   // this was given to the plugin with --dir, or is the default cache directory for the plugin
};

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
    const cEvent *event;             // must be NULL for recordings ; provide data for this event 
    const cRecording *recording;     // must be NULL for events     ; or for this recording
    bool httpImagePaths;             // if true, provide http paths to images
    bool media;                      // if true, provide local filenames for media
//OUT
// Note: tvscraper will clear all output parameters, so you don't have to do this before calling tvscraper
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


//Data structure for live, overview information for each recording / event
// to uniquely identify a recording/event:
// movie + dbid + seasonNumber + episodeNumber (for movies, only dbid is required)
// note: if nothing was found, m_videoType = videoType::none will be returned
class cGetScraperOverview {
public:
  cGetScraperOverview (const cEvent *event = NULL, const cRecording *recording = NULL, std::string *title = NULL, std::string *episodeName = NULL, std::string *IMDB_ID = NULL, cTvMedia *image = NULL, cImageLevels imageLevels = cImageLevels(), cOrientations imageOrientations = cOrientations(), std::string *releaseDate = NULL, std::string *collectionName = NULL):
  m_event(event),
  m_recording(recording),
  m_title(title),
  m_episodeName(episodeName),
  m_IMDB_ID(IMDB_ID),
  m_image(image),
  m_imageLevels(imageLevels),
  m_imageOrientations(imageOrientations),
  m_releaseDate(releaseDate),
  m_collectionName(collectionName)
  {
  }

  bool call(cPlugin *pScraper) {
    m_videoType = eVideoType::none;
    if (!pScraper) return false;
    else return pScraper->Service("GetScraperOverview", this);
  }
//IN: Use constructor, setRequestedImageFormat and setRequestedImageLevel to set these values
  const cEvent *m_event;             // must be NULL for recordings ; provide data for this event
  const cRecording *m_recording;     // must be NULL for events     ; or for this recording
  std::string *m_title;
  std::string *m_episodeName;
  std::string *m_IMDB_ID;
  cTvMedia *m_image;
  cImageLevels m_imageLevels;
  cOrientations m_imageOrientations;
  std::string *m_releaseDate;
  std::string *m_collectionName;
//OUT
// Note: tvscraper will clear all output parameters, so you don't have to do this before calling tvscraper
  eVideoType m_videoType;
  int m_dbid;
  int m_runtime;
// only for movies
  int m_collectionId;
// only for TV shows
  int m_episodeNumber;
  int m_seasonNumber;
};

#endif // __TVSCRAPER_SERVICES_H
