#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

#include <iostream>
#include "stdext.h"
#include "setup.h"
#include "largeString.h"
#include "tools.h"

// STL headers need to be before VDR tools.h (included by <vdr/recording.h>)
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any vdr include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/recording.h>
#include "services.h"

namespace vdrlive {

  // Forward declations from epg_events.h
  class EpgInfo;
  typedef std::shared_ptr<EpgInfo> EpgInfoPtr;

  /**
   *  Some forward declarations
   */
  class RecordingsManager;
  class RecordingsTree;
  class RecordingsTreePtr;
  class RecordingsItem;

  typedef std::shared_ptr<RecordingsManager> RecordingsManagerPtr;
  typedef std::shared_ptr<RecordingsItem> RecordingsItemPtr;
  typedef std::weak_ptr<RecordingsItem> RecordingsItemWeakPtr;

  bool operator< (const RecordingsItemPtr &a, const RecordingsItemPtr &b);
  bool operator< (const std::string &a, const RecordingsItemPtr &b);
  bool operator< (const RecordingsItemPtr &a, const std::string &b);
  bool operator< (int a, const RecordingsItemPtr &b);
  bool operator< (const RecordingsItemPtr &a, int b);

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
                   *	generates a Md5 hash from a cRecording entry. It can be used
                   *  to reidentify a recording.
                   */
                  std::string Md5Hash(cRecording const * recording) const;

                  /**
                   *  fetches a cRecording from VDR's Recordings collection. Returns
                   *  NULL if recording was not found
                   */
                  cRecording const* GetByMd5Hash(std::string const & hash) const;

                  /**
                   *  Move a recording with the given hash according to
                   *  VDRs recording mechanisms.
                   */
                  bool MoveRecording(cRecording const * recording, std::string const & name, bool copy = false) const;

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
                   *	Determine wether the recording has been archived on
                   *	removable media (e.g. DVD-ROM)
                   */
                  static int GetArchiveType(cRecording const * recording);

                  /**
                   *	Provide an identification of the removable media
                   *	(e.g. DVD-ROM Number or Name) where the recording has
                   *	been archived.
                   */
                  static std::string const GetArchiveId(cRecording const * recording, int archiveType);

                  static std::string const GetArchiveDescr(cRecording const * recording);

          private:
                  RecordingsManager();

#if VDRVERSNUM >= 20301
                  static bool StateChanged();
#endif
                  static RecordingsManagerPtr EnsureValidData();

                  static std::weak_ptr<RecordingsManager> m_recMan;
                  static std::shared_ptr<RecordingsTree> m_recTree;
#if VDRVERSNUM >= 20301
                  static cStateKey m_recordingsStateKey;
#else
                  static int m_recordingsState;
#endif

                  cThreadLock m_recordingsLock;
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
  typedef bool (*tCompRec)(const RecordingsItemPtr &a, const RecordingsItemPtr &b);
  class RecordingsItemPtrCompare
  {
    public:
      static bool ByAscendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByDuplicatesName(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByDuplicates(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByDuplicatesLanguage(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByAscendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByDescendingRecordingErrors(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByDescendingDurationDeviation(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByEpisode(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool BySeason(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static bool ByReleaseDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
      static int compareLC(const char *first, const char *second, int *numEqualChars = NULL); // as std::compare, but compare lower case
      static int FindBestMatch(RecordingsItemPtr &BestMatch, const std::vector<RecordingsItemPtr>::const_iterator & First, const std::vector<RecordingsItemPtr>::const_iterator & Last, const RecordingsItemPtr & EPG_Entry);

      static tCompRec getComp(eSortOrder sortOrder);
  };
// search a recording matching an EPG entry. The EPG entry is given with Name, ShortText, Description, Duration, scraperOverview
  bool searchNameDesc(RecordingsItemPtr &RecItem, const std::vector<RecordingsItemPtr> *RecItems, const cEvent *event, cScraperVideo *scraperVideo);

  /**
   *  Base class for entries in recordings tree and recordings list.
   *  All operations possible on recordings are performed against
   *  pointers to instances of recordings items. The C++ polymorphy
   *  delegates them to the 'right' class.
   */
  class RecordingsItem
  {
    friend class RecordingsTree;

    protected:
      RecordingsItem(const std::string& name);

    public:
      virtual ~RecordingsItem();

      virtual time_t StartTime() const = 0;
      virtual bool IsDir() const = 0;
      virtual int Duration() const = 0;
      virtual int DurationDeviation() const { return -1; } // duration deviation in seconds
      virtual int FileSizeMB() const { return -1; }
      virtual const std::string& Name() const { return m_name; }
      virtual const std::string& NameForSearch() const { return m_name_for_search; }
      virtual const char * ShortText() const { return RecInfo()? RecInfo()->ShortText():0; }
      virtual const char * Description() const { return RecInfo()? RecInfo()->Description():0; }
      virtual const std::string Id() const = 0;
      int IdI() const { return m_idI;}
template<class T>
      void AppendShortTextOrDesc(T &target) const;

      virtual const cRecording* Recording() const { return 0; }
      virtual const cRecordingInfo* RecInfo() const { return 0; }

		  void finishRecordingsTree(); // sort recursively, Order: m_cmp_rec (if defined. Otherwise: no sort)
     // dirs: Order: m_cmp_dir (if defined. Otherwise: m_name_for_sort)
      virtual bool operator< (const RecordingsItemPtr &sec) const { return m_name < sec->m_name; }
      virtual bool operator< (const std::string &sec) const { return m_name < sec; }
      virtual bool operator> (const std::string &sec) const { return m_name > sec; }
      virtual bool operator< (int sec) const { return false; }
      virtual bool operator> (int sec) const { return false; }
      virtual int Level() { return 0; }
      int numberOfRecordings();
      RecordingsItemPtr addDirIfNotExists(const std::string &dirName);
		  RecordingsItemPtr addDirCollectionIfNotExists(int collectionId, const cRecording* recording);
		  RecordingsItemPtr addDirSeasonIfNotExists(int collectionId, const cRecording* recording);
		  const std::vector<RecordingsItemPtr> *getRecordings(eSortOrder sortOrder);
		  const std::vector<RecordingsItemPtr> *getDirs() { return &m_subdirs; }
		  bool checkNew();
		  void addDirList(std::vector<std::string> &dirs, const std::string &basePath);

      void setTvShow(const cRecording* recording);
		  void getScraperData(const cRecording* recording, const cImageLevels &imageLevels, std::string *collectionName = NULL);
      bool scraperDataAvailable() const { return m_s_videoType == tMovie || m_s_videoType == tSeries; }
      tvType scraperVideoType() const { return m_s_videoType; }
      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperEpisodeNumber() const { return m_s_episode_number; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      const std::string &scraperName() const { return m_s_title; }
      const std::string &scraperReleaseDate() const { return m_s_release_date; }
      const cTvMedia &scraperImage() const { return m_s_image; }
      int language() const { return m_language; }
      int CompareTexts(const RecordingsItemPtr &second, int *numEqualChars=NULL) const;
      int CompareStD(const RecordingsItemPtr &second, int *numEqualChars=NULL) const;
      bool orderDuplicates(const RecordingsItemPtr &second, bool alwaysShortText, bool lang = false) const;
// To display the recording on the UI
      bool matchesFilter(const std::string &filter);
      virtual const int IsArchived() const { return 0 ; }
      virtual const std::string ArchiveDescr() const { return "" ; }
      virtual const char *NewR() const { return "" ; }
      virtual const int RecordingErrors() const { return -1; }
      virtual int SD_HD() { return m_video_SD_HD; } // < -2: not checked. -1: Radio. 0: SD. 1: HD. >1 UHD or better
      virtual void AppendAsJSArray(cLargeString &target, bool displayFolder) { }
 		  bool recEntriesSorted() { return m_cmp_rec != NULL; }
 		  bool dirEntriesSorted() { return m_cmp_dir != NULL; }

    private:
      std::string GetNameForSearch(std::string const & name);
    protected:
		  int m_idI = -1;
      std::string m_name;
      const std::string m_name_for_search;
      std::vector<RecordingsItemPtr> m_subdirs;
      std::vector<RecordingsItemPtr> m_entries;
		  bool m_entriesSorted = false;
      std::vector<RecordingsItemPtr> m_entries_other_sort;
		  eSortOrder m_sortOrder = (eSortOrder)-1;
		  bool (*m_cmp_dir)(const RecordingsItemPtr &itemPtr1, const RecordingsItemPtr &itemPtr2) = NULL;
		  bool (*m_cmp_rec)(const RecordingsItemPtr &itemPtr1, const RecordingsItemPtr &itemPtr2) = NULL;
// scraper data
      tvType m_s_videoType = tNone;
      int m_s_dbid = 0;
      std::string m_s_title = "";
      std::string m_s_episode_name = "";
      std::string m_s_IMDB_ID = "";
      std::string m_s_release_date = "";
      cTvMedia m_s_image;
      int m_s_runtime = 0;
      int m_s_collection_id = 0;
      int m_s_episode_number = 0;
      int m_s_season_number = 0;
      int m_language = 0;
      int m_video_SD_HD = -2;  // < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better

      int m_duration_deviation = 0;
  };


  /**
   *  A recordings item that resembles a directory with other
   *  subdirectories and/or real recordings.
   */
  class RecordingsItemDir : public RecordingsItem
  {
          public:
                  RecordingsItemDir(const std::string& name, int level);

                  virtual ~RecordingsItemDir();

                  virtual time_t StartTime() const { return 0; }
                  virtual int Duration() const { return -1; }
                  virtual bool IsDir() const { return true; }
                  virtual std::string const Id() const { return ""; }
                  virtual int Level() { return m_level; }

          private:
                  int m_level;
  };

  class RecordingsItemDirSeason : public RecordingsItemDir
  {
          public:
                  RecordingsItemDirSeason(int level, const cRecording* recording);
                  virtual ~RecordingsItemDirSeason();

                  virtual bool operator< (const RecordingsItemPtr &sec) const { return m_s_season_number < sec->scraperSeasonNumber(); }
                  virtual bool operator< (const std::string &sec) const { return false; }
                  virtual bool operator> (const std::string &sec) const { return false; }
                  virtual bool operator< (int sec) const { return m_s_season_number < sec; }
                  virtual bool operator> (int sec) const { return m_s_season_number > sec; }

  };

  class RecordingsItemDirCollection : public RecordingsItemDir
  {
          public:
                  RecordingsItemDirCollection(int level, const cRecording* recording);
                  virtual ~RecordingsItemDirCollection();

                  virtual bool operator< (const RecordingsItemPtr &sec) const { return m_s_collection_id < sec->scraperCollectionId(); }
                  virtual bool operator< (const std::string &sec) const { return false; }
                  virtual bool operator> (const std::string &sec) const { return false; }
                  virtual bool operator< (int sec) const { return m_s_collection_id < sec; }
                  virtual bool operator> (int sec) const { return m_s_collection_id > sec; }

  };


  /**
   *  A recordings item that represents a real recording. This is
   *  the leaf item in the recordings tree or one of the items in
   *  the recordings list.
   */
  class RecordingsItemRec : public RecordingsItem
  {
    public:
      RecordingsItemRec(int idI, const std::string& id, const std::string& name, const cRecording* recording);

      virtual ~RecordingsItemRec();

      virtual time_t StartTime() const { return m_recording->Start(); }
      virtual int Duration() const { return m_recording->FileName() ? m_recording->LengthInSeconds() : -1; } // duration in seconds
      virtual int DurationDeviation() const { return m_duration_deviation; } // duration deviation in seconds
      virtual int FileSizeMB() const { return m_recording->FileName() ? m_recording->FileSizeMB() : -1; } // file size in MB
      virtual bool IsDir() const { return false; }
      virtual const std::string Id() const { return m_id; }

      virtual const cRecording* Recording() const { return m_recording; }
      virtual const cRecordingInfo* RecInfo() const { return m_recording->Info(); }

// To display the recording on the UI
      virtual const int IsArchived() const { return m_isArchived ; }
      virtual const std::string ArchiveDescr() const { return RecordingsManager::GetArchiveDescr(m_recording) ; }
      virtual const char *NewR() const { return LiveSetup().GetMarkNewRec() && (Recording()->GetResume() < 0) ? "_new" : "" ; }
#if VDRVERSNUM >= 20505
      virtual const int RecordingErrors() const { return RecInfo()->Errors(); }
#else
      virtual const int RecordingErrors() const { return -1; }
#endif
      int NumberTsFiles() const { return m_number_ts_files; }
      void AppendRecordingErrorsStr(std::string &target) const;

      virtual int SD_HD();
      virtual void AppendAsJSArray(cLargeString &target, bool displayFolder);
      static void AppendAsJSArray(cLargeString &target, std::vector<RecordingsItemPtr>::const_iterator recIterFirst, std::vector<RecordingsItemPtr>::const_iterator recIterLast, bool &first, const std::string &filter, bool reverse);

    private:
      const cRecording *m_recording;
      const std::string m_id;
      const int m_isArchived;
      int m_number_ts_files;
  };

  /**
   * Class containing recordings to compare or "dummy" recordings, i.e. data from EPG which can be compared with a recording
   */
  class RecordingsItemDummy: public RecordingsItem
  {
    public:
      RecordingsItemDummy(const cEvent *event, cScraperVideo *scraperVideo);
      ~RecordingsItemDummy() { };

      virtual const char * ShortText() const { return m_event->ShortText(); }
      virtual const char * Description() const { return m_event->Description(); }
      virtual time_t StartTime() const { return m_event->StartTime(); }
      virtual int Duration() const { return m_event->Duration() / 60; } // duration in minutes
      virtual bool IsDir() const { return false; }
      virtual std::string const Id() const { return ""; }
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

      RecordingsItemPtr getRoot() const { return m_root; }
      std::vector<std::string> getAllDirs() { std::vector<std::string> result; m_rootFileSystem->addDirList(result, ""); return result; }
      const std::vector<RecordingsItemPtr> *allRecordings() { return &m_allRecordings;}
      const std::vector<RecordingsItemPtr> *allRecordings(eSortOrder sortOrder);

      int MaxLevel() const { return m_maxLevel; }

    private:
      int m_maxLevel;
      RecordingsItemPtr m_root;
      RecordingsItemPtr m_rootFileSystem;
      std::vector<RecordingsItemPtr> m_allRecordings;
      bool m_allRecordingsSorted = false;
      std::vector<RecordingsItemPtr> m_allRecordings_other_sort;
      eSortOrder m_sortOrder = (eSortOrder)-1;
  };


  /**
   *  A smart pointer to a recordings tree. As long as an instance of this
   *  exists the recordings are locked in the vdr.
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
   *	kept alive as long references to it exist.
   */
  RecordingsManagerPtr LiveRecordingsManager();

  inline void print(const char *t, const RecordingsItemPtr &a) {
    std::cout << t << (a ? a->Name() : "nullptr");
  }
  inline void swap(RecordingsItemPtr &a, RecordingsItemPtr &b) {
    a.swap(b);
  }

void AppendScraperData(cLargeString &target, const std::string &s_IMDB_ID, const cTvMedia &s_image, tvType s_videoType, const std::string &s_title, int s_season_number, int s_episode_number, const std::string &s_episode_name, int s_runtime, const std::string &s_release_date);

std::string recordingErrorsHtml(int recordingErrors);

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsLang(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsSd(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
