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

#ifndef DISABLE_TEMPLATES_COLLIDING_WITH_STL
// To get rid of the swap definition in vdr/tools.h
#define DISABLE_TEMPLATES_COLLIDING_WITH_STL
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
  class DuplicatesRecordingsTree;
  class RecordingsItemDir;
  class RecordingsItemDirFileSystem;
  class RecordingsItemRec;
  template<class C> class const_rec_iterator;

  typedef std::shared_ptr<RecordingsTree>           RecordingsTreePtr;
  typedef std::shared_ptr<DuplicatesRecordingsTree> DuplicatesRecordingsTreePtr;
  typedef std::shared_ptr<RecordingsItemDir>        RecordingsItemDirPtr;

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
      enum class eSortOrder { id=0, name=1, date=2, errors=3, durationDeviation=4, duplicatesLanguage=5, list_end=6 };

// static members
    private:
      static inline cStateKey m_recordingsStateKey;
      static inline time_t m_last_recordings_update = 0;
    public:
      static inline std::vector<RecordingsItemDirPtr> dirs_dummy;
      static inline std::vector<RecordingsItemRec*> all_recordings[(int)(eSortOrder::list_end)];
      static inline eSortOrder m_sortOrder = eSortOrder::name;
      static inline bool m_backwards = false;
      static inline std::regex m_filter_regex;
      static inline std::regex *m_filter_regex_ptr = nullptr; // if a filter is provided, this is set to &m_filter_regex

      static inline RecordingsTreePtr m_recTree;
      static inline DuplicatesRecordingsTreePtr m_duplicatesRecTree;

      static void setSortOrder(eSortOrder sortOrder, bool backwards, cSv filter);

      static time_t GetLastRecordingsUpdate() { return m_last_recordings_update; }
      /**
       *  Returns a shared pointer to a fully populated
       *  recordings tree.
       */
      static RecordingsTreePtr GetRecordingsTree();
      static DuplicatesRecordingsTreePtr GetDuplicatesRecordingsTree();

      /**
       *  fetches a cRecording from VDR's Recordings collection. Returns
       *  NULL if recording was not found
       */
      static const cRecording *GetByHash(cSv hash, const cRecordings* Recordings);

      /**
       *  get RecordingsItemRec from the all_recordings. Returns
       *  NULL if recording was not found
       */
      static RecordingsItemRec* const GetByIdHash(cSv hash);

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
       *    2 recording is in use
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

      static void EnsureValidData();
    private:
      static bool StateChanged();
  };

  /**
   * Class containing possible recordings compare functions
   */
  typedef bool (*tCompRec)(const RecordingsItemRec * first, const RecordingsItemRec * second);
  typedef bool (*tCompDir)(const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b);
  class RecordingsItemPtrCompare
  {
    public:
// recs
      static bool ByAscendingDate(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByDuplicatesName(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByDuplicates(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByDuplicatesLanguage(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByAscendingNameDescSort(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByDescendingRecordingErrors(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByDescendingDurationDeviation(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByEpisode(const RecordingsItemRec * first, const RecordingsItemRec * second);
      static bool ByReleaseDate(const RecordingsItemRec * first, const RecordingsItemRec * second);
// dirs
      static bool ByAscendingNameSort(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second);
      static bool BySeason(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second);
// helpers
      static int FindBestMatch(RecordingsItemRec *&BestMatch, const std::vector<RecordingsItemRec*>::const_iterator & First, const std::vector<RecordingsItemRec*>::const_iterator & Last, const RecordingsItemRec* EPG_Entry);

      static tCompRec getComp(RecordingsManager::eSortOrder sortOrder);
  };
// search a recording matching an EPG entry. The EPG entry is given with Name, ShortText, Description, Duration, scraperOverview
  bool searchNameDesc(RecordingsItemRec *&RecItem, const std::vector<RecordingsItemRec*> *RecItems, const cEvent *event, cScraperVideo *scraperVideo);

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
    friend void update_all_recordings_scraper();
    public:
      RecordingsItemRec(const cRecording* recording, cMeasureTime *timeIdentify=nullptr, cMeasureTime *timeOverview=nullptr, cMeasureTime *timeItemRec=nullptr);
      RecordingsItemRec(const cEvent *event, cScraperVideo *scraperVideo); // create dummy

      void finalize(); // call this after constructor, for "work" where cRecording is not required
      bool has_changed(const cRecording* recording); // true if this differs from recording
      void set_seen(bool seen) { m_seen = seen; }
      bool seen() { return m_seen; }

// identify recording
      int Id() const { return m_id;}   // id used by VDR, will not change until VDR restart
      XXH128_hash_t IdHash() const { return m_hash; }  // filename hash, will not change until change of filename
// data from recording / recording->info()
      cSv Name() const { return m_name; }
      cStr NameStr() const { return m_name_str; }  // for strcoll
      cSv ShortText() const { return m_shortText; }
      cSv Description() const { return m_description; }
      cSv ChannelName() const { return m_channelName; }
      cSv Folder() const { return m_folder; }
      int HierarchyLevels() const { return std::count(m_name_vdr.begin(), m_name_vdr.end(), FOLDERDELIMCHAR); }
      double FramesPerSecond(void) const { return m_framesPerSecond; }
      uint16_t FrameWidth(void) const { return m_frameWidth; }
      uint16_t FrameHeight(void) const { return m_frameHeight; }
#if VDRVERSNUM >= 20605
      eScanType ScanType(void) const { return m_scanType; }
      char ScanTypeChar(void) const { return ScanTypeChars[m_scanType]; }
      eAspectRatio AspectRatio(void) const { return m_aspectRatio; }
#endif
      int FileSizeMB() const;    // ändert sich bei Recs, die aufnehmen!
      time_t StartTime() const { return m_startTime; }
      int Duration() const;  // duration in seconds, ändert sich bei Recs, die aufnehmen!
      int RecordingErrors() const;  // ändert sich bei Recs, die aufnehmen! Timestamp info ändert sich

// To display the recording on the UI
      int IsArchived() const { return false; }
      bool IsNew() const { return m_IsNew; }
      const char *NewR() const { return LiveSetup().GetMarkNewRec() && IsNew() ? "_new" : "" ; }
      int NumberTsFiles() const;    // ändert sich bei Recs, die aufnehmen!
// scraper data
      bool scraperDataAvailable() const { return m_s_videoType == tMovie || m_s_videoType == tSeries; }
      tvType scraperVideoType() const { return m_s_videoType; }
      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperEpisodeNumber() const { return m_s_episode_number; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      cSv scraperName() const { return m_s_title; }
      cSv scraperReleaseDate() const { return m_s_release_date; }
      int language() const { return m_language; }
      int SD_HD() const { return m_video_SD_HD; }
      int DurationDeviation() const { return m_scraperVideo?m_scraperVideo->getDurationDeviation():0; } // duration deviation in seconds, ändert sich bei Recs, die aufnehmen!

      bool matches_filter() const; // true if name or short text or descr. match regex of global filter
      bool matches_regex(const std::regex *reg = nullptr) const; // true if name or short text or descr. match regex

    private:
      void getScraperData();
      void getScraperData(const cRecording *recording);
      void updateScraperData(const cRecording *recording);
      int get_SD_HD(const cRecordingInfo *info);
      const cTvMedia &scraperImage() const;
      int CompareTexts(const RecordingsItemRec    *second, int *numEqualChars=NULL) const;
      bool orderDuplicates(const RecordingsItemRec *second, bool alwaysShortText, bool lang = false) const;
    public:
      int CompareStD(const RecordingsItemRec *second, int *numEqualChars=NULL) const;
      void AppendAsJSArray(cToSvConcat<0> &target) const;
      static void AppendAsJSArray(cToSvConcat<0> &target, const_rec_iterator<RecordingsItemRec*> &rec_iterator, bool &first);

      mutable cMeasureTime *m_timeIdentify = nullptr;
      mutable cMeasureTime *m_timeOverview = nullptr;

    private:
      const int m_id;
      const XXH128_hash_t m_hash;
      std::string m_name_vdr;  // name as VDR returns with Name(): folder~name
      const std::string m_file_name; // name as VDR returns with FileName(): name in filesystem, do no sanitize this!
      cSv m_name;
      cStr m_name_str;
      cSv m_folder;
      std::string m_shortText;
      std::string m_description;
      std::string m_channelName;
      mutable int m_fileSizeMB = -1;
      time_t m_startTime = 0;
      mutable int m_duration = -1;
      mutable bool m_IsNew = false;
      mutable int m_recordingErrors = -1;
      double m_framesPerSecond = 0.;
      uint16_t m_frameWidth = 0;
      uint16_t m_frameHeight = 0;
#if VDRVERSNUM >= 20605
      eScanType m_scanType = stUnknown;
      eAspectRatio m_aspectRatio = arUnknown;
#endif
      mutable int m_number_ts_files = -1;
      std::unique_ptr<cScraperVideo_v01> m_scraperVideo;
      tvType m_s_videoType = tNone;
      int m_s_dbid = 0;
      std::string m_s_title;
      std::string m_s_episode_name;
      std::string m_s_IMDB_ID;
      std::string m_s_release_date;
      mutable cTvMedia m_s_image;
      mutable bool m_s_image_requested = false;
      int m_s_runtime = 0;
      int m_s_collection_id = 0;
      int m_s_episode_number = 0;
      int m_s_season_number = 0;
      int m_language = 0;
      int m_video_SD_HD = -2;  // < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better

      bool m_seen;
  };

// iterate over all recordings, sorted by id (VDR-id) =====================================
// examples:
//   a) for (RecordingsItemRec *r: all_recordings_iterator() ) { ... }
//   b) std::vector<RecordingsItemRec*> s_name;
//      s_name.assign(all_recordings_iterator(), all_recordings_iterator(iterator_end() ));
//      std::sort(s_name.begin(), s_name.end(), comp)

  class all_recordings_iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = RecordingsItemRec*;
      using difference_type = std::ptrdiff_t;
      using pointer = RecordingsItemRec**;
      using reference = RecordingsItemRec*&;

      all_recordings_iterator(iterator_begin d):
        m_current(RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].begin()) {
        if (m_current == RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].end() ) return;   // empty container
        if (!*m_current) ++(*this);
      }
      all_recordings_iterator():    // creates the begin() iterator !!!!!
        all_recordings_iterator(iterator_begin() ) { }
      all_recordings_iterator(iterator_end d):
        m_current(RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].end()) {}

      all_recordings_iterator &operator++() {
        for (++m_current; m_current != RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].end(); ++m_current)
          if (*m_current) return *this;
        return *this;
      }
      all_recordings_iterator  operator++(int) { auto tmp = *this; ++*this; return tmp; }
      bool operator==(all_recordings_iterator other) const { return m_current == other.m_current; }
      bool operator!=(all_recordings_iterator other) const { return m_current != other.m_current; }
      reference operator*() { return *m_current; }
      RecordingsItemRec *operator->() { return *m_current; }
    private:
      std::vector<RecordingsItemRec*>::iterator m_current;
  };
inline all_recordings_iterator begin(all_recordings_iterator &it) { return it; }
inline all_recordings_iterator end  (all_recordings_iterator &it) { return all_recordings_iterator(iterator_end() ); }

// C == RecordingsItemRec*   -> iterate over recordings
// C == RecordingsItemDirPtr -> iterate over directories
template<typename C>
  class const_rec_iterator {
    public:
      const_rec_iterator(const std::vector<C> &items, bool backwards, std::regex *regex_filter = nullptr);
      const_rec_iterator(const RecordingsItemDirFileSystem* recordingsItemDirFileSystem, unsigned max_items = std::numeric_limits<unsigned>::max() );
      const_rec_iterator<C> &set_begin();

      const_rec_iterator<C> &set_container(const std::vector<C> &items);
      const_rec_iterator<C> &set_max_items(unsigned max_items);
      const_rec_iterator<C> &set_backwards(bool backwards);
      bool empty() const { return *this == iterator_end(); }
      size_t size();
      const_rec_iterator &operator++();
      bool operator==(iterator_end other) const;
      bool operator!=(iterator_end other) const { return !(*this == other); }
      const C operator*() { return *m_current; }
    private:
      bool current_matches() const; // true if m_current is in dir && matches filter

      bool m_backwards;
      std::regex *m_regex_filter;
      const RecordingsItemDirFileSystem *m_recordingsItemDirFileSystem = nullptr;
      typename std::vector<C>::const_iterator m_begin;
      typename std::vector<C>::const_iterator m_end;
      typename std::vector<C>::const_iterator m_current;
      bool m_backwards_end = false;
      unsigned m_index = 0;  // count actually matching items
      unsigned m_max_items = std::numeric_limits<unsigned>::max();
  };
template<typename C>
inline const_rec_iterator<C> begin(const_rec_iterator<C> &it) { return it; }
template<typename C>
inline iterator_end end(const_rec_iterator<C> &it) { return iterator_end(); }

/**
 *  A recordings item that resembles a directory with other
 *  subdirectories and/or real recordings.
 */
  class RecordingsItemDir
  {
    friend class RecordingsTree;
    friend class DuplicatesRecordingsTree;

    public:
      virtual ~RecordingsItemDir();
      RecordingsItemDir(cSv name, int level);

      cSv Name() const { return m_name; }
      cStr NameStr() const { return m_name; }
      int Level() const { return m_level; }

      void finishRecordingsTree();
// this is called recursively
// recs: sort m_entries, Order: m_cmp_rec (if defined. Otherwise: no sort)
// dirs: sort m_subdirs, Order: m_cmp_dir (if defined. Otherwise: m_name_for_sort)

      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_name < sec->m_name; }
      virtual bool operator< (cSv sec) const { return m_name < sec; }
      virtual bool operator> (cSv sec) const { return m_name > sec; }
      virtual bool operator< (int sec) const { return false; }
      virtual bool operator> (int sec) const { return false; }
      virtual const_rec_iterator<RecordingsItemRec*> get_rec_iterator() const;
      const_rec_iterator<RecordingsItemDirPtr> get_dir_iterator() const;

      virtual int numberOfRecordings() const; // these are the overall recordings, without the regex filter!
      virtual RecordingsItemDirPtr addDirIfNotExists(cSv dirName);
      RecordingsItemDirPtr addDirCollectionIfNotExists(int collectionId, const RecordingsItemRec *rPtr);
      RecordingsItemDirPtr addDirSeasonIfNotExists(int seasonNumber, const RecordingsItemRec *rPtr);
      bool checkNew() const;
      bool matches_regex(std::regex *regex_filter = nullptr) const;
      void addDirList(std::vector<std::string> &dirs, cSv basePath) const;

      void setTvShow(const RecordingsItemRec* rPtr);

      int scraperCollectionId() const { return m_s_collection_id; }
      int scraperSeasonNumber() const { return m_s_season_number; }
      const cTvMedia &scraperImage() const;
      bool dirEntriesSorted() const { return m_cmp_dir != NULL; }

    protected:
      std::string m_name;  // display name
      int m_level;
// other dirs in this dir:
      std::vector<RecordingsItemDirPtr> m_subdirs;
      bool (*m_cmp_dir)(const RecordingsItemDirPtr &itemPtr1, const RecordingsItemDirPtr &itemPtr2) = NULL;
// recordings in this dir:
      mutable std::vector<RecordingsItemRec*> m_entries;
      mutable RecordingsManager::eSortOrder m_sortOrder = (RecordingsManager::eSortOrder)-1;
      bool (*m_cmp_rec)(const RecordingsItemRec* itemPtr1, const RecordingsItemRec* itemPtr2) = NULL;
// scraper data
      const RecordingsItemRec *m_rec_item = nullptr; // in this rec item (if available), there are the relevant scraper data
               // for dirs (collection), it points to a rec item with relevant data for the collection
               // similar for dirs (TV show, season).
               // for others, it is just nullptr -> no data available
      mutable cTvMedia m_s_image;
      mutable bool m_s_image_requested = false;
      cImageLevels m_imageLevels;
      int m_s_collection_id = 0;
      int m_s_season_number = 0;
  };

  class RecordingsItemDirFileSystem : public RecordingsItemDir
  {
    public:
      RecordingsItemDirFileSystem(cSv name, cSv name_contains, int level);
      bool Contains(const RecordingsItemRec *rec) const;
      bool Contains(const RecordingsItemDirPtr &r) const { return true; };
      RecordingsItemDirPtr addDirIfNotExists(cSv dirName);
      const_rec_iterator<RecordingsItemRec*> get_rec_iterator() const;
      int numberOfRecordings() const;
    private:
      std::string m_name_contains;  // all recordings with this folder name are in this folder
  };
  class RecordingsItemDirFlat : public RecordingsItemDir
  {
    public:
      RecordingsItemDirFlat(): RecordingsItemDir(cSv(), 0) {}
      const_rec_iterator<RecordingsItemRec*> get_rec_iterator() const;
  };
  class RecordingsItemDirSeason : public RecordingsItemDir
  {
    public:
      RecordingsItemDirSeason(int level, const RecordingsItemRec *rPtr);

      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_s_season_number < sec->scraperSeasonNumber(); }
      virtual bool operator< (cSv sec) const { return false; }
      virtual bool operator> (cSv sec) const { return false; }
      virtual bool operator< (int sec) const { return m_s_season_number < sec; }
      virtual bool operator> (int sec) const { return m_s_season_number > sec; }
      const_rec_iterator<RecordingsItemRec*> get_rec_iterator() const;
  };

  class RecordingsItemDirCollection : public RecordingsItemDir
  {
    public:
      RecordingsItemDirCollection(int level, const RecordingsItemRec *rPtr);

      virtual bool operator< (const RecordingsItemDirPtr &sec) const { return m_s_collection_id < sec->scraperCollectionId(); }
      virtual bool operator< (cSv sec) const { return false; }
      virtual bool operator> (cSv sec) const { return false; }
      virtual bool operator< (int sec) const { return m_s_collection_id < sec; }
      virtual bool operator> (int sec) const { return m_s_collection_id > sec; }
      const_rec_iterator<RecordingsItemRec*> get_rec_iterator() const;
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
      const std::vector<RecordingsItemRec*> *allRecordings() { return &RecordingsManager::all_recordings[(int)(RecordingsManager::m_sortOrder)];}
      const std::vector<RecordingsItemRec*> *allRecordings(RecordingsManager::eSortOrder sortOrder);

      int MaxLevel() const { return m_maxLevel; }

    private:
      int m_maxLevel;
      RecordingsItemDirPtr m_root;
      RecordingsItemDirPtr m_rootFileSystem;
  };

  /**
   *  The duplicates recordings tree contains all duplicate recordings in a
   *  tree like fashion.
   */
  class DuplicatesRecordingsTree
  {
    friend class RecordingsManager;

    private:
      DuplicatesRecordingsTree(RecordingsTreePtr &recordingsTree);

    public:
      RecordingsItemDirPtr getRoot() const { return m_root; }
      RecordingsItemDirPtr getFlatRoot() const { return m_flat_root; }

    private:
      RecordingsItemDirPtr m_root;
      RecordingsItemDirPtr m_flat_root;
  };

  inline void swap(RecordingsItemDirPtr &a, RecordingsItemDirPtr &b) {
    a.swap(b);
  }

void AppendScraperData(cToSvConcat<0> &target, cSv s_IMDB_ID, const cTvMedia &s_image, tvType s_videoType, cSv s_title, int s_season_number, int s_episode_number, cSv s_episode_name, int s_runtime, cSv s_release_date);

std::string recordingErrorsHtml(int recordingErrors);

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsLang(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);
void addDuplicateRecordingsSd(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);

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
