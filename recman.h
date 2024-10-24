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

template<std::size_t N>
cToSvConcat<N> & StringAppendFrameParams(cToSvConcat<N> &s, const cRecording *rec) {
#if VDRVERSNUM >= 20605
  if (!rec || ! rec->Info() ) return s;
  if (rec->Info()->FrameWidth() && rec->Info()->FrameHeight() ) {
    s << rec->Info()->FrameWidth() << 'x' << rec->Info()->FrameHeight();
    if (rec->Info()->FramesPerSecond() > 0) {
      s.append("/");
      s.appendFormated("%.2g", rec->Info()->FramesPerSecond() );
      if (rec->Info()->ScanType() != stUnknown)
        s.append(1, rec->Info()->ScanTypeChar());
     }
     if (rec->Info()->AspectRatio() != arUnknown)
       s << ' ' << rec->Info()->AspectRatioText();
  }
#endif
  return s;
}

  // Forward declarations from epg_events.h
  class EpgInfo;
  typedef std::shared_ptr<EpgInfo> EpgInfoPtr;

  /**
   *  Some forward declarations
   */
  class RecordingsManager;
  class RecordingsTree;
  class RecordingsTreePtr;
  class RecordingsItemDir;
  class RecordingsItemRec;

  typedef std::shared_ptr<RecordingsManager> RecordingsManagerPtr;
  typedef std::shared_ptr<RecordingsItemDir> RecordingsItemDirPtr;
  typedef std::shared_ptr<RecordingsItemRec> RecordingsItemRecPtr;

  bool operator< (const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b);
  bool operator< (cSv a, const RecordingsItemDirPtr &b);
  bool operator< (const RecordingsItemDirPtr &a, cSv b);
  bool operator< (int a, const RecordingsItemDirPtr &b);
  bool operator< (const RecordingsItemDirPtr &a, int b);

  int GetNumberOfTsFiles(const cRecording* recording);

  /**
   *  Class for managing recordings inside the live plugin. It
   *  provides some convenience methods and provides automatic
   *  locking during requests on the recordings or during the
   *  traversion of the recordings tree or lists, which can only be
   *  obtained through methods of the RecordingsManager.
   */
  class RecordingsManager
  {
    friend RecordingsManagerPtr LiveRecordingsManager();

    public:
      /**
       *  Returns a shared pointer to a fully populated
       *  recordings tree.
       */
      RecordingsTreePtr GetRecordingsTree() const;

      /**
       *  fetches a cRecording from VDR's Recordings collection. Returns
       *  NULL if recording was not found
       */
      cRecording const* GetByMd5Hash(cSv hash) const;

      /**
       *  fetches a cRecording from the RecordingsTree collection. Returns
       *  NULL if recording was not found
       */
      RecordingsItemRecPtr const GetByIdHash(cSv hash) const;

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
      bool UpdateRecording(cRecording const * recording, cSv directory, cSv name, bool copy, cSv title, cSv shorttext, cSv description) const;

      /**
       *  Delete recording resume with the given hash according to
       *  VDRs recording mechanisms.
       */
      void DeleteResume(cRecording const * recording) const;

      /**
       *  Delete recording marks with the given hash according to
       *  VDRs recording mechanisms.
       */
      void DeleteMarks(cRecording const * recording) const;

      /**
       *  Delete a recording with the given hash according to
       *  VDRs recording deletion mechanisms.
       */
      void DeleteRecording(cRecording const * recording) const;

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

    private:
      RecordingsManager();

      static bool StateChanged();
      static RecordingsManagerPtr EnsureValidData();

      static std::weak_ptr<RecordingsManager> m_recMan;
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
    public:
      RecordingsItemRec(cSv name, const cRecording* recording, cMeasureTime *timeIdentify, cMeasureTime *timeOverview, cMeasureTime *timeImage, cMeasureTime *timeDurationDeviation, cMeasureTime *timeNumTsFiles, cMeasureTime *timeItemRec);

      virtual ~RecordingsItemRec();

      const cSv Name() const { return m_name; }
      const cSv NameForSearch() const { return m_name_for_search; }
      virtual const char * ShortText() const { return RecInfo()? RecInfo()->ShortText():0; }
      virtual const char * Description() const { return RecInfo()? RecInfo()->Description():0; }
      virtual time_t StartTime() const { return m_recording->Start(); }
      virtual int Duration() const { return m_recording->FileName() ? m_recording->LengthInSeconds() : -1; } // duration in seconds
      virtual int DurationDeviation() const { return m_duration_deviation; } // duration deviation in seconds
      virtual int FileSizeMB() const { return m_recording->FileName() ? m_recording->FileSizeMB() : -1; } // file size in MB
      int IdI() const { return m_idI;}
      virtual const XXH128_hash_t IdHash() const { return m_hash; }

      virtual const cRecording* Recording() const { return m_recording; }
      virtual const cRecordingInfo* RecInfo() const { return m_recording->Info(); }

// To display the recording on the UI
      virtual const int IsArchived() const { return m_isArchived; }
      virtual const std::string ArchiveDescr() const { return RecordingsManager::GetArchiveDescr(m_recording) ; }
      virtual const char *NewR() const { return LiveSetup().GetMarkNewRec() && Recording()->IsNew() ? "_new" : "" ; }
      virtual bool checkNew() const { return m_recording->IsNew(); }  // for recursive checks on dirs, here we don't check LiveSetup
#if VDRVERSNUM >= 20505
      virtual const int RecordingErrors() const { return RecInfo()->Errors(); }
#else
      virtual const int RecordingErrors() const { return -1; }
#endif
      virtual int NumberTsFiles() const {
        if (m_number_ts_files == -2) m_number_ts_files = GetNumberOfTsFiles(m_recording);
        return m_number_ts_files;
      }
      virtual void getScraperData(std::string *collectionName = NULL);
      bool scraperDataAvailable() const { return m_s_videoType == tMovie || m_s_videoType == tSeries; }
      tvType scraperVideoType() const { return m_s_videoType; }
      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperEpisodeNumber() const { return m_s_episode_number; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      const cSv scraperName() const { return m_s_title; }
      const cSv scraperReleaseDate() const { return m_s_release_date; }
      const cTvMedia &scraperImage() const;
      int language() const { return m_language; }
      int CompareTexts(const RecordingsItemRecPtr &second, int *numEqualChars=NULL) const;
      int CompareStD(const RecordingsItemRecPtr &second, int *numEqualChars=NULL) const;
      bool orderDuplicates(const RecordingsItemRecPtr &second, bool alwaysShortText, bool lang = false) const;
// To display the recording on the UI
      bool matchesFilter(cSv filter) const;

      virtual int SD_HD() const;
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
      const cRecording *m_recording = nullptr;
      const XXH128_hash_t m_hash;
      const int m_isArchived = 0;
      mutable int m_number_ts_files = -2;
      std::unique_ptr<cScraperVideo> m_scraperVideo;
      tvType m_s_videoType = tNone;
      int m_s_dbid = 0;
      std::string m_s_title = "";
      std::string m_s_episode_name = "";
      std::string m_s_IMDB_ID = "";
      std::string m_s_release_date = "";
      mutable cTvMedia m_s_image;
      mutable bool m_s_image_requested = false;
      cImageLevels m_imageLevels;
      int m_s_runtime = 0;
      int m_s_collection_id = 0;
      int m_s_episode_number = 0;
      int m_s_season_number = 0;
      int m_language = 0;
      mutable int m_video_SD_HD = -2;  // < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better

      int m_duration_deviation = 0;
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
    public:
      RecordingsItemDummy(const cEvent *event, cScraperVideo *scraperVideo);

      virtual time_t StartTime() const { return m_event->StartTime(); }
      virtual int Duration() const { return m_event->Duration() / 60; } // duration in minutes
      virtual const char * ShortText() const { return m_event->ShortText(); }
      virtual const char * Description() const { return m_event->Description(); }
      virtual int DurationDeviation() const { return -2; }
      virtual int FileSizeMB() const { return -1; }

      virtual const cRecording* Recording() const { return nullptr; }
      virtual const cRecordingInfo* RecInfo() const { return nullptr; }

      virtual bool checkNew() const { return false; }
      virtual const int IsArchived() const { return 0 ; }
      virtual const std::string ArchiveDescr() const { return std::string(); }
      virtual const char *NewR() const { return ""; }
      virtual const int RecordingErrors() const { return -1; }
      virtual int NumberTsFiles() const { return 0 ; }
      virtual void getScraperData(std::string *collectionName = NULL) {}
      const cTvMedia &scraperImage() const { return m_s_image; }

      virtual int SD_HD() const { return 0; }
      virtual void AppendAsJSArray(cToSvConcat<0> &target) const {}
    private:
      const cEvent *m_event;
  };

  /**
   *  The recordings tree contains all recordings in a file system
   *  tree like fashion.
   */
  class RecordingsTree
  {
    friend class RecordingsManager;

    private:
      RecordingsTree(RecordingsManagerPtr recManPtr);

    public:
      virtual ~RecordingsTree();

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


  /**
   *  A smart pointer to a recordings tree. As long as an instance of this
   *  exists the recordings are locked in the VDR.
   */
  class RecordingsTreePtr : public std::shared_ptr<RecordingsTree>
  {
          friend class RecordingsManager;

          private:
                  RecordingsTreePtr(RecordingsManagerPtr recManPtr, std::shared_ptr<RecordingsTree> recTree);

          public:
                  RecordingsTreePtr();
                  virtual ~RecordingsTreePtr();

          private:
                  RecordingsManagerPtr m_recManPtr;
  };

  /**
   *  return singleton instance of RecordingsManager as a shared Pointer.
   *  This ensures that after last use of the RecordingsManager it is
   *  deleted. After deletion of the original RecordingsManager a repeated
   *  call to this function creates a new RecordingsManager which is again
   *  kept alive as long references to it exist.
   */
  RecordingsManagerPtr LiveRecordingsManager();

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

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
