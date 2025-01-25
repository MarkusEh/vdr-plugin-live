#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

// STL headers need to be before VDR tools.h (included by <vdr/recording.h>)
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <list>

#if TNTVERSION >= 30000
  #include <cxxtools/log.h>  // must be loaded before any VDR include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <iostream>
#include "stdext.h"
#include "setup.h"

#include <vdr/recording.h>
#include <vdr/channels.h>
#include "stringhelpers.h"
#include "tools.h"
#include "services.h"

namespace vdrlive {

  /**
   *  Some forward declarations
   */
  class RecordingsTree;
  class RecordingsItemDir;
  class RecordingsItemRec;

  typedef std::shared_ptr<RecordingsTree>    RecordingsTreePtr;
  typedef std::shared_ptr<RecordingsItemDir> RecordingsItemDirPtr;
  typedef std::shared_ptr<RecordingsItemRec> RecordingsItemRecPtr;

  bool operator< (const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b);
  bool operator< (cSv a, const RecordingsItemDirPtr &b);
  bool operator< (const RecordingsItemDirPtr &a, cSv b);
  bool operator< (int a, const RecordingsItemDirPtr &b);
  bool operator< (const RecordingsItemDirPtr &a, int b);

  int GetNumberOfTsFiles(int recId);

  /**
   *  Class for managing recordings inside the live plugin. It
   *  provides some convenience methods and provides automatic
   *  locking during requests on the recordings or during the
   *  traversion of the recordings tree or lists, which can only be
   *  obtained through methods of the RecordingsManager.
   */
  class RecordingsManager
  {
    public:
      /**
       *  Returns a shared pointer to a fully populated
       *  recordings tree.
       */
      static RecordingsTreePtr GetRecordingsTree();

      /**
       *  fetches a cRecording from VDR's Recordings collection. Returns
       *  NULL if recording was not found
       */
      static const cRecording *GetByHash(cSv hash, const cRecordings* Recordings);

      /**
       *  fetches a cRecording from the RecordingsTree collection. Returns
       *  NULL if recording was not found
       */
      static RecordingsItemRecPtr const GetByIdHash(cSv hash);

      /**
       *  Move a recording with the given hash according to
       *  VDRs recording mechanisms.
       *  @param directory new name of the sub folder this recording is stored in.
       *  @param name new recording folder name.
       *  @param copy create a copy of the original recording rather than moving it.
       *  @param title new title of the recording.
       *  @param shorttext new short text of the recording.
       *  @param description new description of the recording.
       */
      static bool UpdateRecording(cSv hash, cSv directory, cSv name, bool copy, cSv title, cSv shorttext, cSv description);

      /**
       *  Delete recording resume with the given hash according to
       *  VDRs recording mechanisms.
       */
      static void DeleteResume(cRecording const * recording);

      /**
       *  Delete recording marks with the given hash according to
       *  VDRs recording mechanisms.
       */
      static void DeleteMarks(cRecording const * recording);

      /**
       *  Delete a recording with the given hash according to
       *  VDRs recording deletion mechanisms.
       *  If name is provided, it is set to recording->Name()  (name for use in Menues)
       *  return:
       *    0 success
       *    1 no recording with recording_hash exists (name will not be provided ...)
       *    2 recoring is in use
       *    3 other error (recording->Delete() returned false)
       */
      static int DeleteRecording(cSv recording_hash, std::string *name = nullptr);

      /**
       *  Determine whether the recording has been archived on
       *  removable media (e.g. DVD-ROM)
       */
      static int GetArchiveType(cRecording const * recording);

      /**
       *  Provide an identification of the removable media
       *  (e.g. DVD-ROM Number or Name) where the recording has
       *  been archived.
       */
      static std::string const GetArchiveId(cRecording const * recording, int archiveType);

      static std::string const GetArchiveDescr(cRecording const * recording);

      /**
       *  Is this recording currently played?
       *  Return codes:
       *    0: this recording is currently played
       *    1: this recording does not exist
       *    2: no recording is played
       *    3: another recording is played
       *  Also return *fileName=recording->FileName() if requested
       */
      static int CheckReplay(cSv recording_hash, std::string *fileName = nullptr);

    private:
      static bool StateChanged();
      static void EnsureValidData();

      static std::shared_ptr<RecordingsTree> m_recTree;
      static cStateKey m_recordingsStateKey;
      static time_t m_last_recordings_update;
  };

  class ShortTextDescription
  {
    public:
      ShortTextDescription(const char * ShortText, const char * Description);
      wint_t getNextNonPunctChar();
    private:
      const char * m_short_text;
      const char * m_description;
  };

  /**
   * Class containing possible recordings compare functions
   */
  enum class eSortOrder { name, date, errors, durationDeviation, duplicatesLanguage };
  typedef bool (*tCompRec)(const RecordingsItemRecPtr &a, const RecordingsItemRecPtr &b);
  typedef bool (*tCompDir)(const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b);
  class RecordingsItemPtrCompare
  {
    public:
// recs
      static bool ByAscendingDate(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByDuplicatesName(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByDuplicates(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByDuplicatesLanguage(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByAscendingNameDescSort(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByDescendingRecordingErrors(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByDescendingDurationDeviation(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByEpisode(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
      static bool ByReleaseDate(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second);
// dirs
      static bool ByAscendingNameSort(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second);
      static bool BySeason(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second);
// helpers
      static int FindBestMatch(RecordingsItemRecPtr &BestMatch, const std::vector<RecordingsItemRecPtr>::const_iterator & First, const std::vector<RecordingsItemRecPtr>::const_iterator & Last, const RecordingsItemRecPtr & EPG_Entry);

      static tCompRec getComp(eSortOrder sortOrder);
  };
// search a recording matching an EPG entry. The EPG entry is given with Name, ShortText, Description, Duration, scraperOverview
  bool searchNameDesc(RecordingsItemRecPtr &RecItem, const std::vector<RecordingsItemRecPtr> *RecItems, const cEvent *event, cScraperVideo *scraperVideo);

/**
 *  A recordings item that resembles a directory with other
 *  subdirectories and/or real recordings.
 */
  class RecordingsItemDir
  {
    friend class RecordingsTree;

    public:
      virtual ~RecordingsItemDir();
      RecordingsItemDir(cSv name, int level);

      cSv Name() const { return m_name; }
      int Level() const { return m_level; }

      void finishRecordingsTree(); // sort recursively, Order: m_cmp_rec (if defined. Otherwise: no sort)
// dirs: Order: m_cmp_dir (if defined. Otherwise: m_name_for_sort)
      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_name < sec->m_name; }
      virtual bool operator< (cSv sec) const { return m_name < sec; }
      virtual bool operator> (cSv sec) const { return m_name > sec; }
      virtual bool operator< (int sec) const { return false; }
      virtual bool operator> (int sec) const { return false; }
      int numberOfRecordings() const;
      RecordingsItemDirPtr addDirIfNotExists(cSv dirName);
      RecordingsItemDirPtr addDirCollectionIfNotExists(int collectionId, const RecordingsItemRecPtr &rPtr);
      RecordingsItemDirPtr addDirSeasonIfNotExists(int seasonNumber, const RecordingsItemRecPtr &rPtr);
      const std::vector<RecordingsItemRecPtr> *getRecordings(eSortOrder sortOrder);
      const std::vector<RecordingsItemDirPtr> *getDirs() { return &m_subdirs; }
      bool checkNew() const;
      void addDirList(std::vector<std::string> &dirs, cSv basePath) const;

      void setTvShow(const RecordingsItemRecPtr &rPtr);

      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      const cTvMedia &scraperImage() const;
      bool recEntriesSorted() const { return m_cmp_rec != NULL; }
      bool dirEntriesSorted() const { return m_cmp_dir != NULL; }

    protected:
      std::string m_name;
      int m_level;
      std::vector<RecordingsItemDirPtr> m_subdirs;
      std::vector<RecordingsItemRecPtr> m_entries;
      bool m_entriesSorted = false;
      std::vector<RecordingsItemRecPtr> m_entries_other_sort;
      eSortOrder m_sortOrder = (eSortOrder)-1;
      bool (*m_cmp_dir)(const RecordingsItemDirPtr &itemPtr1, const RecordingsItemDirPtr &itemPtr2) = NULL;
      bool (*m_cmp_rec)(const RecordingsItemRecPtr &itemPtr1, const RecordingsItemRecPtr &itemPtr2) = NULL;
// scraper data
      RecordingsItemRecPtr m_rec_item; // in this rec item (if available), there are the relevant scraper data
               // for dirs (collection), it points to a rec item with relevant data for the collection
               // similar for dirs (TV show, season).
               // for others, it is just nullptr -> no data available
      mutable cTvMedia m_s_image;
      mutable bool m_s_image_requested = false;
      cImageLevels m_imageLevels;
      int m_s_collection_id = 0;
      int m_s_season_number = 0;
  };

  class RecordingsItemDirSeason : public RecordingsItemDir
  {
    public:
      RecordingsItemDirSeason(int level, const RecordingsItemRecPtr &rPtr);
      virtual ~RecordingsItemDirSeason();

      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_s_season_number < sec->scraperSeasonNumber(); }
      virtual bool operator< (cSv sec) const { return false; }
      virtual bool operator> (cSv sec) const { return false; }
      virtual bool operator< (int sec) const { return m_s_season_number < sec; }
      virtual bool operator> (int sec) const { return m_s_season_number > sec; }
  };

  class RecordingsItemDirCollection : public RecordingsItemDir
  {
    public:
      RecordingsItemDirCollection(int level, const RecordingsItemRecPtr &rPtr);
      virtual ~RecordingsItemDirCollection();

      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_s_collection_id < sec->scraperCollectionId(); }
      virtual bool operator< (cSv sec) const { return false; }
      virtual bool operator> (cSv sec) const { return false; }
      virtual bool operator< (int sec) const { return m_s_collection_id < sec; }
      virtual bool operator> (int sec) const { return m_s_collection_id > sec; }
  };


  /**
   *  A recordings item that represents a real recording. This is
   *  the leaf item in the recordings tree or one of the items in
   *  the recordings list.
   */
  class RecordingsItemRec
  {
    friend class RecordingsTree;
    friend class RecordingsItemDir;
    friend class RecordingsItemDirSeason;
    friend class RecordingsItemDirCollection;
    friend class RecordingsItemPtrCompare;
    public:
      RecordingsItemRec(cSv name, const cRecording* recording, cMeasureTime *timeIdentify, cMeasureTime *timeOverview, cMeasureTime *timeImage, cMeasureTime *timeDurationDeviation, cMeasureTime *timeNumTsFiles, cMeasureTime *timeItemRec);

      virtual ~RecordingsItemRec();

      cSv Name() const { return m_name; }
      cSv NameForSearch() const { return m_name_for_search; }
      const char *ShortText() const { return m_shortText.c_str(); }
      const char *Description() const { return m_description.c_str(); }
      cSv ChannelName() const { return m_channelName; }
      const char *Folder() const { return *m_Folder; }
      int FileSizeMB() const { return m_fileSizeMB; }
      time_t StartTime() const { return m_startTime; }
      int Duration() const { return m_duration; } // duration in seconds
      int DurationDeviation() const { return m_duration_deviation; } // duration deviation in seconds
      int IdI() const { return m_idI;}
      XXH128_hash_t IdHash() const { return m_hash; }

// To display the recording on the UI
      int IsArchived() const { return m_isArchived; }
      bool checkNew() const { return m_checkNew; }
      const char *NewR() const { return LiveSetup().GetMarkNewRec() && checkNew() ? "_new" : "" ; }
      int RecordingErrors() const { return m_recordingErrors; }
      int NumberTsFiles() const {
        if (m_number_ts_files == -2) m_number_ts_files = GetNumberOfTsFiles(m_idI);
        return m_number_ts_files;
      }
      bool scraperDataAvailable() const { return m_s_videoType == tMovie || m_s_videoType == tSeries; }
      tvType scraperVideoType() const { return m_s_videoType; }
      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperEpisodeNumber() const { return m_s_episode_number; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      cSv scraperName() const { return m_s_title; }
      cSv scraperReleaseDate() const { return m_s_release_date; }
      int language() const { return m_language; }
      int SD_HD() const { return m_video_SD_HD; }
      double FramesPerSecond(void) const { return m_framesPerSecond; }
      uint16_t FrameWidth(void) const { return m_frameWidth; }
      uint16_t FrameHeight(void) const { return m_frameHeight; }
      eScanType ScanType(void) const { return m_scanType; }
      char ScanTypeChar(void) const { return ScanTypeChars[m_scanType]; }
      eAspectRatio AspectRatio(void) const { return m_aspectRatio; }

      int CompareStD(const RecordingsItemRecPtr &second, int *numEqualChars=NULL) const;
      bool matchesFilter(cSv filter) const;
    private:
      void getScraperData(const cRecording *recording);
      int get_SD_HD(const cRecordingInfo *info);
      const cTvMedia &scraperImage() const;
      int CompareTexts(const RecordingsItemRecPtr &second, int *numEqualChars=NULL) const;
      bool orderDuplicates(const RecordingsItemRecPtr &second, bool alwaysShortText, bool lang = false) const;
    public:
      virtual void AppendAsJSArray(cToSvConcat<0> &target) const;
      static void AppendAsJSArray(cToSvConcat<0> &target, std::vector<RecordingsItemRecPtr>::const_iterator recIterFirst, std::vector<RecordingsItemRecPtr>::const_iterator recIterLast, bool &first, cSv filter, bool reverse);

      mutable cMeasureTime *m_timeIdentify = nullptr;
      mutable cMeasureTime *m_timeOverview = nullptr;
      mutable cMeasureTime *m_timeImage = nullptr;
      mutable cMeasureTime *m_timeDurationDeviation = nullptr;

    protected:
      const std::string m_name;
      std::string GetNameForSearch(cSv name);
      const std::string m_name_for_search;
      const int m_idI = -1;
      const XXH128_hash_t m_hash;
      const int m_isArchived = 0;
      mutable int m_number_ts_files = -2;
      std::unique_ptr<cScraperVideo> m_scraperVideo;
      tvType m_s_videoType = tNone;
      int m_s_dbid = 0;
      std::string m_s_title;
      std::string m_s_episode_name;
      std::string m_s_IMDB_ID;
      std::string m_s_release_date;
      mutable cTvMedia m_s_image;
      mutable bool m_s_image_requested = false;
      cImageLevels m_imageLevels;
      int m_s_runtime = 0;
      int m_s_collection_id = 0;
      int m_s_episode_number = 0;
      int m_s_season_number = 0;
      int m_language = 0;
      int m_video_SD_HD = -2;  // < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better

      int m_duration_deviation = 0;
// to remove m_recording
      std::string m_shortText;
      std::string m_description;
      std::string m_channelName;
      cString m_Folder;
      int m_fileSizeMB = -1;
      time_t m_startTime = 0;
      int m_duration = -1;
      bool m_checkNew = false;
      int m_recordingErrors = 0;
      double m_framesPerSecond = 0.;
      uint16_t m_frameWidth = 0;
      uint16_t m_frameHeight = 0;
      eScanType m_scanType = stUnknown;
      eAspectRatio m_aspectRatio = arUnknown;

    protected:
      RecordingsItemRec(cSv name):
        m_name(name),
        m_name_for_search(GetNameForSearch(name)),
        m_hash(XXH3_128bits("abc", 3)) {}
  };

  /**
   * Class containing recordings to compare or "dummy" recordings, i.e. data from EPG which can be compared with a recording
   */
  class RecordingsItemDummy: public RecordingsItemRec
  {
    friend class RecordingsItemPtrCompare;
    public:
      RecordingsItemDummy(const cEvent *event, cScraperVideo *scraperVideo);
  };

  /**
   *  The recordings tree contains all recordings in a file system
   *  tree like fashion.
   */
  class RecordingsTree
  {
    friend class RecordingsManager;

    private:
      RecordingsTree();

    public:
      ~RecordingsTree();

      RecordingsItemDirPtr getRoot() const { return m_root; }
      std::vector<std::string> getAllDirs() { std::vector<std::string> result; m_rootFileSystem->addDirList(result, ""); return result; }
      const std::vector<RecordingsItemRecPtr> *allRecordings() { return &m_allRecordings;}
      const std::vector<RecordingsItemRecPtr> *allRecordings(eSortOrder sortOrder);

      int MaxLevel() const { return m_maxLevel; }
      time_t getCreationTimestamp() { return m_creation_timestamp; }

    private:
      int m_maxLevel;
      RecordingsItemDirPtr m_root;
      RecordingsItemDirPtr m_rootFileSystem;
      std::vector<RecordingsItemRecPtr> m_allRecordings;
      bool m_allRecordingsSorted = false;
      std::vector<RecordingsItemRecPtr> m_allRecordings_other_sort;
      eSortOrder m_sortOrder = (eSortOrder)-1;
      time_t m_creation_timestamp = 0;
  };

  inline void print(const char *t, const RecordingsItemDirPtr &a) {
    std::cout << t << (a ? a->Name() : "nullptr");
  }
  inline void swap(RecordingsItemDirPtr &a, RecordingsItemDirPtr &b) {
    a.swap(b);
  }
  inline void swap(RecordingsItemRecPtr &a, RecordingsItemRecPtr &b) {
    a.swap(b);
  }

void AppendScraperData(cToSvConcat<0> &target, cSv s_IMDB_ID, const cTvMedia &s_image, tvType s_videoType, cSv s_title, int s_season_number, int s_episode_number, cSv s_episode_name, int s_runtime, cSv s_release_date);

std::string recordingErrorsHtml(int recordingErrors);

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsLang(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsSd(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);

template<std::size_t N>
cToSvConcat<N> & StringAppendFrameParams(cToSvConcat<N> &s, const RecordingsItemRec *itemRec) {
#if VDRVERSNUM >= 20605
  if (!itemRec) return s;
  if (itemRec->FrameWidth() && itemRec->FrameHeight() ) {
    s << itemRec->FrameWidth() << 'x' << itemRec->FrameHeight();
    if (itemRec->FramesPerSecond() > 0) {
      s.appendFormated("/%.2g", itemRec->FramesPerSecond() );
      if (itemRec->ScanType() != stUnknown)
        s.append(1, itemRec->ScanTypeChar());
     }
     if (itemRec->AspectRatio() != arUnknown)
       s << ' ' << AspectRatioTexts[itemRec->AspectRatio()];
  }
#endif
  return s;
}
} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
