#ifndef __TVSCRAPER_SERVICES_H
#define __TVSCRAPER_SERVICES_H
#include <vdr/epg.h>
#include <vdr/recording.h>
#include <vdr/plugin.h>
#include <string>
#include <vector>
#include <memory>

/*********************************************************************
* Helper Structures
*********************************************************************/
enum tvType {
    tSeries,
    tMovie,
    tNone,
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

// Data structures for full series and episode information
//     service  "GetMovie"
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

// Data structure for full series and episode information
//     service  "GetSeries"
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

// Data structure for service "GetPosterBannerV2"
class ScraperGetPosterBannerV2 {
public:
    ScraperGetPosterBannerV2(void) {
        type = tNone;
        event = NULL;
        recording = NULL;
    };
// in
    const cEvent *event;             // check type for this event
    const cRecording *recording;     // check type for this recording
//out
    tvType type;                     //typeSeries or typeMovie
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

// Data structure for enviromment ("GetEnvironment" call)
class cEnvironment
{
  public:
//in: nothing, no input required
//out
    std::string basePath;  // All images are in this path or subdirectories. This was given to the plugin with --dir, or is the default cache directory for the plugin.
    std::string seriesPath;
    std::string moviesPath;
};

// NEW interface, used by live =========================================================

// Data structure for service "GetScraperImageDir"
class cGetScraperImageDir {
public:
//in: nothing, no input required
//out
  std::string scraperImageDir;   // this was given to the plugin with --dir, or is the default cache directory for the plugin. It will always end with a '/'
  cPlugin *call(cPlugin *pScraper = NULL) {
    if (!pScraper) return cPluginManager::CallFirstService("GetScraperImageDir", this);
    else return pScraper->Service("GetScraperImageDir", this)?pScraper:NULL;
  }
};

// Data structure for service "GetScraperUpdateTimes"
class cGetScraperUpdateTimes {
public:
//in: nothing, no input required
//out
  time_t m_EPG_UpdateTime;
  time_t m_recordingsUpdateTime;
  cPlugin *call(cPlugin *pScraper = NULL) {
    if (!pScraper) return cPluginManager::CallFirstService("GetScraperUpdateTimes", this);
    else return pScraper->Service("GetScraperUpdateTimes", this)?pScraper:NULL;
  }
};

// ===========================================================================
// the following enums & classes are for cScraperVideo (see below)
// ===========================================================================

enum class eCharacterType {
  director = 1, // Regisseur
  writer = 2,   // Autor
  actor = 3,
  guestStar = 4,
  crew = 5,
  creator = 6,
  producer = 7,
  showrunner = 8,
  musicalGuest = 9,
  host = 10,
  executiveProducer = 11,
  screenplay = 21,            // Drehbuchautor
  originalMusicComposer = 31, // Komponist
  others = 51,
};

class cCharacter {
  public:
    virtual eCharacterType getType() = 0;
    virtual const std::string &getPersonName() = 0;     // "real name" of the person
    virtual const std::string &getCharacterName() = 0;  // name of character in video
    virtual const cTvMedia &getImage() = 0;
    virtual ~cCharacter() {}
};

// ===========================================================================
// the following enums & classes are for cScraperVideo->getImage(...)  (see below)
// you can use them to select the desired orientation and "image level" (see below) of the image returned
// ===========================================================================

enum class eOrientation {
  none = 0,
  banner = 1,
  landscape = 2,
  portrait = 3,
};

// class cOrientations: combine orientations, with priorities.
// Example: You want a landscape, but, as fallback, also accept a banner. If none of these is available, even a portrait is better than no image:
// cOrientations(eOrientation::landscape, eOrientation::banner, eOrientation::portrait);
class cOrientations {
  public:
    cOrientations(eOrientation first = eOrientation::none, eOrientation second = eOrientation::none, eOrientation third = eOrientation::none) {
      m_orientations = (int)first | (int(second)<<3) | (int(third)<<6);
    }
  private:
    friend class cOrientationsInt;
    int m_orientations;
};

enum class eImageLevel {
  none = 0,
  episodeMovie = 1,      // for TV Shows: episode. For movies: movie (itself)
  seasonMovie  = 2,      // for TV Shows: season.  For movies: movie (itself)
  tvShowCollection = 3,  // for TV Shows: itself.  For movies: collection
  anySeasonCollection = 4, // for TV Shows: any season. For movies: collection
};

// class cImageLevels: combine image levels, with priorities.
// example: image for event/recording:  cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::TV_ShowCollection, eImageLevel::anySeasonCollection)
// if you don't want the episode still: cImageLevels(eImageLevel::seasonMovie, eImageLevel::TV_ShowCollection, eImageLevel::anySeasonCollection)
class cImageLevels {
  public:
    cImageLevels(eImageLevel first = eImageLevel::none, eImageLevel second = eImageLevel::none, eImageLevel third = eImageLevel::none, eImageLevel forth = eImageLevel::none){
      m_imageLevels = (int)first | (int(second)<<3) | (int(third)<<6) | (int(forth)<<9);
    }
  private:
    friend class cImageLevelsInt;
    int m_imageLevels;
};

// The following class will be returned by the service handler method cGetScraperVideo
// note: the event/recording object used to create an instance must be valid during all method calls.
// VDR will grant such validity for about 5 seconds.

// if you don't need a specific information, just pass NULL
// parameter fullPath: If this is false, the image paths are relative to the path returned by "GetScraperImageDir"
class cScraperVideo
{
  public:
// during creation of this instance, a movie/series/episode is identified
// with the following methods you can request the "IDs of the identified object"
    virtual tvType getVideoType() = 0;  // if this is tNone, nothing was identified by scraper. Still, some information (image, duration deviation ...) might be available
    virtual int getDbId() = 0;          // if > 0: TMDB (themoviedb) ID; if < 0: tvdb (thetvdb) ID
    virtual int getEpisodeNumber() = 0; // return 0 if episode was not identified
    virtual int getSeasonNumber() = 0;  // return 0 if episode was not identified

// getOverview provides the "most important" attributes
    virtual bool getOverview(std::string *title, std::string *episodeName, std::string *releaseDate, int *runtime, std::string *imdbId, int *collectionId, std::string *collectionName = NULL) = 0; // return false if no scraper data is available

// "single image, or several images"
    virtual cTvMedia getImage(cImageLevels imageLevels = cImageLevels(), cOrientations imageOrientations = cOrientations(), bool fullPath = true) = 0;
    virtual std::vector<cTvMedia> getImages(eOrientation orientation, int maxImages = 3, bool fullPath = true) = 0;

// "characters, including actors"
    virtual std::vector<std::unique_ptr<cCharacter>> getCharacters(bool fullPath = true) = 0;

// other attributes, available even if getVideoType() == tNone
    virtual int getDurationDeviation() = 0;
    virtual int getHD() = 0;  // 0: SD. 1: HD. 2: UHD
    virtual int getLanguage() = 0;  // return -1 in case of errors

// the other attributes of a movie or TV show:
// note: runtime will be provided here only for movies. For tv shows, the runtime is provided with getEpisode
    virtual bool getMovieOrTv(std::string *title, std::string *originalTitle, std::string *tagline, std::string *overview, std::vector<std::string> *genres, std::string *homepage, std::string *releaseDate, bool *adult, int *runtime, float *popularity, float *voteAverage, int *voteCount, std::vector<std::string> *productionCountries, std::string *imdbId, int *budget, int *revenue, int *collectionId, std::string *collectionName, std::string *status, std::vector<std::string> *networks, int *lastSeason) = 0;

// episode attributes. return true if getVideoType() == tSeries && episode is identified
    virtual bool getEpisode(std::string *name, std::string *overview, int *absoluteNumber, std::string *firstAired, int *runtime, float *voteAverage, int *voteCount, std::string *imdbId) = 0;
    virtual ~cScraperVideo() {}
};

// service to return cScraperVideo instance
class cGetScraperVideo {
public:
  cGetScraperVideo(const cEvent *event = NULL, const cRecording *recording = NULL):
    m_event(event), m_recording(recording) { }

  cPlugin *call(cPlugin *pScraper = NULL) {
    if (!pScraper) return cPluginManager::CallFirstService("GetScraperVideo", this);
    return pScraper->Service("GetScraperVideo", this)?pScraper:NULL;
  }
//IN: Use constructor to set these values
  const cEvent *m_event;             // must be NULL for recordings ; provide data for this event
  const cRecording *m_recording;     // must be NULL for events     ; or for this recording
//OUT
   std::unique_ptr<cScraperVideo> m_scraperVideo;
};

#endif // __TVSCRAPER_SERVICES_H
