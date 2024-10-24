#ifdef HAVE_PCRE2
#include "StringMatch.h"
#endif
#include <iostream>
#include <dirent.h>

#include "recman.h"
#include "tools.h"
#include "services.h"
#include "epg_events.h"


// STL headers need to be before VDR tools.h (included by <vdr/videodir.h>)
#include <fstream>
#include <stack>
#include <algorithm>
#include <limits>

#include <vdr/videodir.h>
#include <vdr/config.h>

#define INDEXFILESUFFIX   "/index.vdr"
#define LENGTHFILESUFFIX  "/length.vdr"

namespace vdrlive {

  /**
   *  Implementation of class RecordingsManager:
   */
  std::weak_ptr<RecordingsManager> RecordingsManager::m_recMan;
  std::shared_ptr<RecordingsTree> RecordingsManager::m_recTree;
  cStateKey RecordingsManager::m_recordingsStateKey;
  time_t RecordingsManager::m_last_recordings_update = 0;

  // The RecordingsManager holds a VDR lock on the
  // Recordings. Additionally the singleton instance of
  // RecordingsManager is held in a weak pointer. If it is not in
  // use any longer, it will be freed automatically, which leads to a
  // release of the VDR recordings lock. Upon requesting access to
  // the RecordingsManager via LiveRecordingsManger function, first
  // the weak ptr is locked (obtaining a std::shared_ptr from an possible
  // existing instance) and if not successful a new instance is
  // created, which again locks the VDR Recordings.
  //
  // RecordingsManager provides factory methods to obtain other
  // recordings data structures. The data structures returned keep if
  // needed the instance of RecordingsManager alive until destructed
  // themselves. This way the use of LIVE::recordings is straight
  // forward and does hide the locking needs from the user.

  RecordingsManager::RecordingsManager()
  {
  }

  RecordingsTreePtr RecordingsManager::GetRecordingsTree() const
  {
    RecordingsManagerPtr recMan = EnsureValidData();
    if (! recMan) {
      esyslog("live, ERROR, recMan == nullptr after call to EnsureValidData");
      return RecordingsTreePtr(recMan, std::shared_ptr<RecordingsTree>());
    }
    return RecordingsTreePtr(recMan, m_recTree);
  }

  cRecording const * RecordingsManager::GetByMd5Hash(cSv hash) const
  {
    if (hash.length() != 42) return 0;
    if (hash.compare(0, 10, "recording_") != 0) return 0;
    XXH128_hash_t xxh = parse_hex_128(hash.substr(10));
    LOCK_RECORDINGS_READ;
    for (cRecording* rec = (cRecording *)Recordings->First(); rec; rec = (cRecording *)Recordings->Next(rec)) {
      XXH128_hash_t xxh_rec = XXH3_128bits(rec->FileName(), strlen(rec->FileName()));
      if (xxh_rec.high64 == xxh.high64 && xxh_rec.low64 == xxh.low64) return rec;
    }
    return 0;
  }

  RecordingsItemRecPtr const RecordingsManager::GetByIdHash(cSv hash) const
  {
    if (hash.length() != 42) return 0;
    if (hash.compare(0, 10, "recording_") != 0) return 0;
    XXH128_hash_t xxh = parse_hex_128(hash.substr(10));
    for (RecordingsItemRecPtr recItem : *LiveRecordingsManager()->GetRecordingsTree()->allRecordings()) {
      XXH128_hash_t xxh_rec = recItem->IdHash();
      if (xxh_rec.high64 == xxh.high64 && xxh_rec.low64 == xxh.low64) return recItem;
    }
    return 0;
  }

  bool RecordingsManager::UpdateRecording(cRecording const * recording, cSv directory, cSv name, bool copy, cSv title, cSv shorttext, cSv description) const
  {
    if (!recording)
      return false;

    std::string oldname = recording->FileName();
    size_t found = oldname.find_last_of("/");

    if (found == std::string::npos)
      return false;

    std::string filename = FileSystemExchangeChars(directory.empty() ? name : cSv(cToSvReplace(directory, "/", "~") << "~" << name), true);

    // Check for injections that try to escape from the video dir.
    if (filename.compare(0, 3, "..~") == 0 || filename.find("~..") != std::string::npos) {
      esyslog("live: renaming failed: new name invalid \"%.*s\"", (int)filename.length(), filename.data());
      return false;
    }

    std::string newname = concat(cVideoDirectory::Name(), "/", filename, cSv(oldname).substr(found));

    if (!MoveDirectory(oldname, newname, copy)) {
      esyslog("live: renaming failed from '%.*s' to '%s'", (int)oldname.length(), oldname.data(), newname.c_str());
      return false;
    }

    LOCK_RECORDINGS_WRITE;
    if (!copy)
      Recordings->DelByName(oldname.c_str());
    Recordings->AddByName(newname.c_str());
    recording = Recordings->GetByName(newname.c_str());   // old pointer to recording invalid after DelByName/AddByName
    cRecordingUserCommand::InvokeCommand(*cString::sprintf("rename \"%s\"", *strescape(oldname.c_str(), "\\\"$'")), newname.c_str());

#if VDRVERSNUM >= 20502
// 2021-04-06: Version 2.5.2
// Made the functions cRecordingInfo::SetData() and cRecordingInfo::SetAux() public
// da ist auch cRecordingInfo *Info(void) const { return info; }
// in 2.5.1: noch const cRecordingInfo *Info(void) const { return info; }

    // update texts
    // need null terminated strings for VDR API
    std::string desc(description);
    desc.erase(std::remove(desc.begin(), desc.end(), '\r'), desc.end()); // remove \r from HTML

    cRecordingInfo* info = recording->Info();
    if (title != cSv(info->Title()) || shorttext != cSv(info->ShortText()) || desc != cSv(info->Description()))
    {
      info->SetData(title.empty() ? nullptr : std::string(title).c_str(), shorttext.empty() ? nullptr : std::string(shorttext).c_str(), desc.empty() ? nullptr : desc.c_str());
      info->Write();
    }
#endif

    return true;
  }

  void RecordingsManager::DeleteResume(cRecording const * recording) const
  {
    if (!recording)
      return;

    cResumeFile ResumeFile(recording->FileName(), recording->IsPesRecording());
    ResumeFile.Delete();
  }

  void RecordingsManager::DeleteMarks(cRecording const * recording) const
  {
    if (!recording)
      return;

    cMarks marks;
    marks.Load(recording->FileName());
    if (marks.Count()) {
      cMark *mark = marks.First();
      while (mark) {
        cMark *nextmark = marks.Next(mark);
        marks.Del(mark);
        mark = nextmark;
      }
      marks.Save();
    }
  }

  void RecordingsManager::DeleteRecording(cRecording const * recording) const
  {
    if (!recording)
      return;

    std::string name(recording->FileName());
    const_cast<cRecording *>(recording)->Delete();
    LOCK_RECORDINGS_WRITE;
    Recordings->DelByName(name.c_str());
  }

  int RecordingsManager::GetArchiveType(cRecording const * recording)
  {
// 1: on DVD
// 2: on HDD
// 0: "normal" VDR recording
    if (!LiveSetup().GetUseArchive() ) return 0;
    if (!recording || !recording->FileName() ) return 0;
    size_t folder_length = strlen(recording->FileName());
    char file[folder_length + 9];   // "/dvd.vdr" + 0 terminator -> 9
    memcpy(file, recording->FileName(), folder_length);
    memcpy(file + folder_length, "/dvd.vdr", 8);
    file[folder_length + 8] = 0;
    struct stat buffer;
    if (stat (file, &buffer) == 0) return 1;
    memcpy(file + folder_length, "/hdd.vdr", 8);
    if (stat (file, &buffer) == 0) return 2;
// stat is 10% faster than access on my system. On others, there is a larger difference
// see https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
    return 0;
  }

  std::string const RecordingsManager::GetArchiveId(cRecording const * recording, int archiveType)
  {
    std::string filename = recording->FileName();

    if (archiveType==1) {
      std::string dvdFile = filename + "/dvd.vdr";
      std::ifstream dvd(dvdFile);

    if (dvd) {
      std::string archiveDisc;
      std::string videoDisc;
      dvd >> archiveDisc;
      if ("0000" == archiveDisc) {
        dvd >> videoDisc;
        return videoDisc;
      }
      return archiveDisc;
    }
    } else if(archiveType==2) {
      std::string hddFile = filename + "/hdd.vdr";
      std::ifstream hdd(hddFile);

      if (hdd) {
        std::string archiveDisc;
        hdd >> archiveDisc;
        return archiveDisc;
      }
    }
    return "";
  }

  std::string const RecordingsManager::GetArchiveDescr(cRecording const * recording)
  {
    int archiveType;
    std::string archived;
    archiveType = GetArchiveType(recording);
    if (archiveType==1) {
      archived += " [";
      archived += tr("On archive DVD No.");
      archived += ": ";
      archived += GetArchiveId(recording, archiveType);
      archived += "]";
    } else if (archiveType==2) {
      archived += " [";
      archived += tr("On archive HDD No.");
      archived += ": ";
      archived += GetArchiveId(recording, archiveType);
      archived += "]";
    }
    return archived;
  }

  bool RecordingsManager::StateChanged ()
  {
    bool result = false;

    // will return true only, if the Recordings List has been changed since last read
    if (cRecordings::GetRecordingsRead(m_recordingsStateKey)) {
      result = true;
      m_recordingsStateKey.Remove();
    }

    return result;
  }

  RecordingsManagerPtr RecordingsManager::EnsureValidData()
  {
    // Get singleton instance of RecordingsManager.  'this' is not
    // an instance of std::shared_ptr of the singleton
    // RecordingsManager, so we obtain it in the overall
    // recommended way.
    RecordingsManagerPtr recMan = LiveRecordingsManager();
    if (! recMan) {
      // theoretically this code is never reached ...
      esyslog("live: lost RecordingsManager instance while using it!");
      recMan = RecordingsManagerPtr();
    }
    if (m_last_recordings_update + 1 > time(NULL) ) return recMan; // don't update too often

    // StateChanged must be executed every time, so not part of
    // the short cut evaluation in the if statement below.
    bool stateChanged = StateChanged();
// check: changes on scraper data?
    cGetScraperUpdateTimes scraperUpdateTimes;
    if (!stateChanged && scraperUpdateTimes.call(LiveSetup().GetPluginTvscraper()) )
      stateChanged = scraperUpdateTimes.m_recordingsUpdateTime > m_last_recordings_update;

    if (stateChanged || (!m_recTree) ) {
      m_last_recordings_update = time(NULL);
      std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
      m_recTree = std::shared_ptr<RecordingsTree>(new RecordingsTree(recMan));
      std::chrono::duration<double> timeNeeded = std::chrono::high_resolution_clock::now() - begin;
      dsyslog("live: DH: ------ RecordingsTree::RecordingsTree() --------, required time: %9.5f", timeNeeded.count() );
      if (!m_recTree) {
        esyslog("live: ERROR creation of recordings tree failed!");
        return RecordingsManagerPtr();
      }

    }
    return recMan;
  }

  ShortTextDescription::ShortTextDescription(const char * ShortText, const char * Description):
    m_short_text(ShortText),
    m_description(Description)
  {  }
  wint_t ShortTextDescription::getNextNonPunctChar() {
    wint_t result;
    do {
      if( m_short_text && *m_short_text ) result = getNextUtfCodepoint(m_short_text);
      else result = getNextUtfCodepoint(m_description);
    } while (result && std::ispunct<wchar_t>(result, g_locale));
    return std::tolower<wchar_t>(result, g_locale);
  }

  /**
   * Implementation of class RecordingsItemPtrCompare
   */

  int RecordingsItemPtrCompare::FindBestMatch(RecordingsItemRecPtr & BestMatch, const std::vector<RecordingsItemRecPtr>::const_iterator & First, const std::vector<RecordingsItemRecPtr>::const_iterator & Last, const RecordingsItemRecPtr & EPG_Entry) {
// d: length of movie in minutes, without commercial breaks
// Assumption: the maximum length of commercial breaks is cb% * d
// Assumption: cb = 34%
   const long cb = 34;
// lengthLowerBound: minimum of d, if EPG_Entry->Duration() has commercial breaks
   long lengthLowerBound = EPG_Entry->Duration() * 100 / ( 100 + cb) ;
// lengthUpperBound: if EPG_Entry->Duration() is d (no commercial breaks), max length of recording with commercial breaks
// Add VDR recording margins to this value
   long lengthUpperBound = EPG_Entry->Duration() * (100 + cb) / 100;
   lengthUpperBound += (::Setup.MarginStart + ::Setup.MarginStop) * 60;
   if(EPG_Entry->Duration() >= 90*60 && lengthLowerBound < 70*60) lengthLowerBound = 70*60;

   int numRecordings = 0;
   int min_deviation = std::numeric_limits<int>::max();
   std::vector<RecordingsItemRecPtr>::const_iterator bestMatchIter;
   for ( std::vector<RecordingsItemRecPtr>::const_iterator iter = First; iter != Last; ++iter)
     if ( (*iter)->Duration() >= lengthLowerBound && (*iter)->Duration() <= lengthUpperBound ) {
        int deviation = abs( (*iter)->Duration() - EPG_Entry->Duration() );
        if (deviation < min_deviation || numRecordings == 0) {
          min_deviation = deviation;
          bestMatchIter = iter;
        }
        ++numRecordings;
   }
   if (numRecordings > 0) BestMatch = *bestMatchIter;
   return numRecordings;
}

  bool RecordingsItemPtrCompare::ByAscendingDate(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second)
  {
    return (first->StartTime() < second->StartTime());
  }

  bool RecordingsItemPtrCompare::ByDuplicatesName(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second)  // return first < second
  {
           return first->orderDuplicates(second, false);
  }

  bool RecordingsItemPtrCompare::ByDuplicates(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second)  // return first < second
  {
          return first->orderDuplicates(second, true);
  }

  bool RecordingsItemPtrCompare::ByDuplicatesLanguage(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second)  // return first < second
  {
          return first->orderDuplicates(second, true, true);
  }

  size_t firstNonPunct(cSv s) {
// returns first non-punct char in s
    size_t ret;
    for (ret = 0; ret < s.length() && std::ispunct(s[ret]); ret++ );
    return ret;
  }
  int compareWithLocale(cSv first, cSv second) {
    size_t start_f = firstNonPunct(first );
    size_t start_s = firstNonPunct(second);
    if (start_f >= first.length() && start_s >= second.length() ) return 0;
    if (start_f >= first.length() ) return 1;
    if (start_s >= second.length() ) return -1;
// see https://en.cppreference.com/w/cpp/locale/collate/compare
// Compares the character sequence [low1, high1) to the character sequence [low2, high2)
    int i = g_collate_char.compare(&first[start_f], &first[0] + first.length(),
                                  &second[start_s], &second[0] + second.length() );
    return i;
  }
  bool RecordingsItemPtrCompare::ByAscendingNameDescSort(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second)  // return first < second
// used for sort
  {
    int i = compareWithLocale(first->Name(), second->Name());
    if(i != 0) return i < 0;
    return first->CompareStD(second) < 0;
  }
  bool RecordingsItemPtrCompare::ByAscendingNameSort(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second)  // return first < second
  {
    return compareWithLocale(first->Name(), second->Name()) < 0;
  }

  bool RecordingsItemPtrCompare::ByDescendingRecordingErrors(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second){
     return first->RecordingErrors() > second->RecordingErrors();
  }
  bool RecordingsItemPtrCompare::ByDescendingDurationDeviation(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second) {
    return first->DurationDeviation() > second->DurationDeviation();
  }

  bool RecordingsItemPtrCompare::ByEpisode(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second) {
    return first->scraperEpisodeNumber() < second->scraperEpisodeNumber();
  }

  bool RecordingsItemPtrCompare::BySeason(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second) {
    return first->scraperSeasonNumber() < second->scraperSeasonNumber();
  }

  bool RecordingsItemPtrCompare::ByReleaseDate(const RecordingsItemRecPtr & first, const RecordingsItemRecPtr & second) {
    return first->scraperReleaseDate() < second->scraperReleaseDate();
  }

  tCompRec RecordingsItemPtrCompare::getComp(eSortOrder sortOrder) {
    switch (sortOrder) {
      case eSortOrder::name: return &RecordingsItemPtrCompare::ByAscendingNameDescSort;
      case eSortOrder::date: return &RecordingsItemPtrCompare::ByAscendingDate;
      case eSortOrder::errors: return &RecordingsItemPtrCompare::ByDescendingRecordingErrors;
      case eSortOrder::durationDeviation: return &RecordingsItemPtrCompare::ByDescendingDurationDeviation;
      case eSortOrder::duplicatesLanguage: return &RecordingsItemPtrCompare::ByDuplicatesLanguage;
    }
    esyslog("live: ERROR, RecordingsItemPtrCompare::getComp, sortOrder %d unknown", (int)sortOrder);
          return &RecordingsItemPtrCompare::ByAscendingNameDescSort;
  }


bool searchNameDesc(RecordingsItemRecPtr &RecItem, const std::vector<RecordingsItemRecPtr> *RecItems, const cEvent *event, cScraperVideo *scraperVideo) {
  if (RecItems->empty() ) return false;  // there are no recordings

// find all recordings with equal name
  RecordingsItemRecPtr dummy = std::make_shared<RecordingsItemDummy>(event, scraperVideo);
  const auto equalName = std::equal_range(RecItems->begin(), RecItems->end(), dummy, RecordingsItemPtrCompare::ByDuplicatesName);
  if (equalName.first == equalName.second) return false; // there is no recording with this name

// find all recordings with matching short text / description / language
  auto equalDuplicates = std::equal_range(equalName.first, equalName.second, dummy, RecordingsItemPtrCompare::ByDuplicatesLanguage);

  if (equalDuplicates.first == equalDuplicates.second) {   // nothing found. Try again, and include other languages
    equalDuplicates = std::equal_range(equalName.first, equalName.second, dummy, RecordingsItemPtrCompare::ByDuplicates);
  }

  if (equalDuplicates.first != equalDuplicates.second) {   // exact match found
    if (RecordingsItemPtrCompare::FindBestMatch(RecItem, equalDuplicates.first, equalDuplicates.second, dummy) > 0) return true;
    RecItem = *equalDuplicates.first;
    return true;
  }
// no exact match found, get recording with most matching characters in short text / description
  int numEqualCharsLow = 0;
  int numEqualCharsUp  = 0;

  if (equalDuplicates.first != equalName.first) {
    --equalDuplicates.first;
    (*equalDuplicates.first)->CompareStD(dummy, &numEqualCharsLow);
  }
  if (equalDuplicates.second != equalName.second)
    (*equalDuplicates.second)->CompareStD(dummy, &numEqualCharsUp);

  if (numEqualCharsLow > numEqualCharsUp ) {
    if (numEqualCharsLow > 5) { RecItem = *equalDuplicates.first; return true; }
  } else {
    if (numEqualCharsUp  > 5) { RecItem = *equalDuplicates.second;  return true; }
  }

// no sufficient match in short text / description
// get best match from length of event match
  int num_match_rec = RecordingsItemPtrCompare::FindBestMatch(RecItem, equalName.first, equalName.second, dummy);
  if (num_match_rec > 5) return false; // series (too many matching lengths)
  if (num_match_rec > 0) return true;
// no matching length
  RecItem = *equalDuplicates.first;
  return !scraperVideo;
}

  /**
   *  Implementation of class RecordingsItemDir:
   */
  RecordingsItemDir::RecordingsItemDir(cSv name, int level):
    m_name(name),
    m_level(level) { }

  RecordingsItemDir::~RecordingsItemDir() { }

  int RecordingsItemDir::numberOfRecordings() const {
    int result = m_entries.size();
    for (const auto &item: m_subdirs) result += item->numberOfRecordings();
    return result;
  }

  void RecordingsItemDir::finishRecordingsTree() {
    for (auto &item: m_subdirs) item->finishRecordingsTree();
    if (m_cmp_rec) std::sort(m_entries.begin(), m_entries.end(), m_cmp_rec);
    if (m_cmp_dir) std::sort(m_subdirs.begin(), m_subdirs.end(), m_cmp_dir);
    else std::sort(m_subdirs.begin(), m_subdirs.end(), RecordingsItemPtrCompare::ByAscendingNameSort);
    m_entries.shrink_to_fit();
    m_subdirs.shrink_to_fit();
  }

  RecordingsItemDirPtr RecordingsItemDir::addDirIfNotExists(cSv dirName) {
    std::vector<RecordingsItemDirPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), dirName);
    if (iter != m_subdirs.end() && !(dirName < *iter) ) return *iter;
    RecordingsItemDirPtr dirPtr = std::make_shared<RecordingsItemDir>(dirName, Level() + 1);
    m_subdirs.insert(iter, dirPtr);
    return dirPtr;
  }

  RecordingsItemDirPtr RecordingsItemDir::addDirCollectionIfNotExists(int collectionId, const RecordingsItemRecPtr &rPtr) {
    std::vector<RecordingsItemDirPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), collectionId);
    if (iter != m_subdirs.end() && !(collectionId < *iter) ) return *iter;
    RecordingsItemDirPtr dirPtr2 = std::make_shared<RecordingsItemDirCollection>(Level() + 1, rPtr);
    m_subdirs.insert(iter, dirPtr2);
    return dirPtr2;
  }

  RecordingsItemDirPtr RecordingsItemDir::addDirSeasonIfNotExists(int seasonNumber, const RecordingsItemRecPtr &rPtr) {
    std::vector<RecordingsItemDirPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), seasonNumber);
    if (iter != m_subdirs.end() && !(seasonNumber < *iter) ) return *iter;
    RecordingsItemDirPtr dirPtr2 = std::make_shared<RecordingsItemDirSeason>(Level() + 1, rPtr);
    m_subdirs.insert(iter, dirPtr2);
    return dirPtr2;
  }

  const std::vector<RecordingsItemRecPtr> *RecordingsItemDir::getRecordings(eSortOrder sortOrder)
  {
    if (m_cmp_rec) return &m_entries;
    if (sortOrder == eSortOrder::name) {
      if (!m_entriesSorted) {
        std::sort(m_entries.begin(), m_entries.end(), RecordingsItemPtrCompare::getComp(eSortOrder::name));
        m_entriesSorted = true;
      }
       return &m_entries;
    }
    if (m_sortOrder == sortOrder) return &m_entries_other_sort;
    if (m_entries_other_sort.empty() ) m_entries_other_sort = m_entries;
    std::sort(m_entries_other_sort.begin(), m_entries_other_sort.end(), RecordingsItemPtrCompare::getComp(sortOrder));
    m_sortOrder = sortOrder;
    return &m_entries_other_sort;
  }

  bool RecordingsItemDir::checkNew() const{
    for (const auto &rec:m_entries) if (rec->checkNew()) return true;
    for (const auto &subdir:m_subdirs) if (subdir->checkNew() ) return true;
    return false;
  }
  void RecordingsItemDir::addDirList(std::vector<std::string> &dirs, cSv basePath) const
  {
    std::string basePath0(basePath);
    if (basePath.empty() ) dirs.push_back("");
    else basePath0.append("/");
    size_t basePath0_len = basePath0.length();
    for (const auto &subdir: m_subdirs) {
      basePath0.erase(basePath0_len);
      basePath0.append(subdir->m_name);
      dirs.push_back(basePath0);
      subdir->addDirList(dirs, basePath0);
    }
  }

  void RecordingsItemDir::setTvShow(const RecordingsItemRecPtr &rPtr) {
    if (m_cmp_dir) return;
    m_cmp_dir = RecordingsItemPtrCompare::BySeason;
    m_imageLevels = cImageLevels(eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection);
    m_rec_item = rPtr;
    m_s_season_number = m_rec_item->scraperSeasonNumber();
  }

  const cTvMedia &RecordingsItemDir::scraperImage() const {
    if (!m_s_image_requested) {
      m_s_image_requested = true;
      if (m_rec_item && m_rec_item->m_scraperVideo)
        m_s_image = m_rec_item->m_scraperVideo->getImage(m_imageLevels, cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner), false);
    }
    return m_s_image;
  }

  /**
   *  Implementation of class RecordingsItemDirCollection:
   */
  RecordingsItemDirCollection::RecordingsItemDirCollection(int level, const RecordingsItemRecPtr &rPtr):
    RecordingsItemDir("", level)
  {
    m_cmp_rec = RecordingsItemPtrCompare::ByReleaseDate;
    m_imageLevels = cImageLevels(eImageLevel::tvShowCollection, eImageLevel::seasonMovie);
    m_rec_item = rPtr;
    m_s_collection_id = m_rec_item->m_s_collection_id;
    m_rec_item->m_scraperVideo->getOverview(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &m_name);
  }

  RecordingsItemDirCollection::~RecordingsItemDirCollection() { }

  /**
   *  Implementation of class RecordingsItemDirSeason:
   */
  RecordingsItemDirSeason::RecordingsItemDirSeason(int level, const RecordingsItemRecPtr &rPtr):
    RecordingsItemDir(cToSvInt(rPtr->m_s_season_number), level)
  {
    m_cmp_rec = RecordingsItemPtrCompare::ByEpisode;
//    m_imageLevels = cImageLevels(eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection);
    m_imageLevels = cImageLevels(eImageLevel::seasonMovie);
    m_rec_item = rPtr;
    m_s_season_number = m_rec_item->m_s_season_number;
  }
  RecordingsItemDirSeason::~RecordingsItemDirSeason() { }

  int GetNumberOfTsFiles(const cRecording* recording) {
// find our number of ts files
    if (!recording || !recording->FileName() ) return -1;
    size_t folder_length = strlen(recording->FileName());
    cToSvConcat file_path(recording->FileName(), "/00001.ts");
    struct stat buffer;
    uint32_t num_ts_files;
    std::chrono::time_point<std::chrono::high_resolution_clock> timeStart = std::chrono::high_resolution_clock::now();
    for (num_ts_files = 1; num_ts_files < 100000u; ++num_ts_files) {
      file_path.erase(folder_length+1);
      file_path.appendInt<5>(num_ts_files).append(".ts");
      if (stat (file_path.c_str(), &buffer) != 0) break;
    }
    std::chrono::duration<double> timeNeeded = std::chrono::high_resolution_clock::now() - timeStart;
    if (timeNeeded.count() > 0.1)
      dsyslog("live, time GetNumberOfTsFiles: %f, recording %s, num ts files %d", timeNeeded.count(), recording->FileName(), num_ts_files - 1);
    return num_ts_files - 1;
  }

  /**
   *  Implementation of class RecordingsItemRec:
   */
  RecordingsItemRec::RecordingsItemRec(cSv name, const cRecording* recording, cMeasureTime *timeIdentify, cMeasureTime *timeOverview, cMeasureTime *timeImage, cMeasureTime *timeDurationDeviation, cMeasureTime *timeNumTsFiles, cMeasureTime *timeItemRec):
    m_name(name),
    m_name_for_search(GetNameForSearch(name)),
    m_idI(recording->Id()),
    m_recording(recording),
    m_hash(XXH3_128bits(recording->FileName(), strlen(recording->FileName()) )),
    m_isArchived(RecordingsManager::GetArchiveType(m_recording) )
  {
// dsyslog("live: REC: C: rec %s -> %s", name.c_str(), parent->Name().c_str());
    timeItemRec->start();
    m_timeIdentify = timeIdentify;
    m_timeOverview = timeOverview;
    m_timeImage = timeImage;
    m_timeDurationDeviation = timeDurationDeviation;

    m_imageLevels = cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection);
// this is for the image in "overview". We allow as many levels as we have, to ensure that there is an image
//  m_imageLevels = cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::anySeasonCollection);
    getScraperData();
    timeItemRec->stop();
  }

  RecordingsItemRec::~RecordingsItemRec()
  {
    // dsyslog("live: REC: D: rec %s", Name().c_str());
  }

  std::string RecordingsItemRec::GetNameForSearch(cSv name)
  {
    std::string result;
    result.reserve(name.length());
    const char *name_c = name.data();
    const char *name_c_end = name_c + name.length();
    while(name_c < name_c_end) {
      wint_t codepoint = getNextUtfCodepoint(name_c);
      if(!std::ispunct<wchar_t>(codepoint, g_locale) ) stringAppendUtfCodepoint(result, std::tolower<wchar_t>(codepoint, g_locale));
    }
    return result;
  }

  bool RecordingsItemRec::matchesFilter(cSv filter) const {
    if (filter.empty() ) return true;
#ifdef HAVE_PCRE2
    StringMatch sm(filter);
    return sm.Matches(Name()) or
      sm.Matches(ShortText()?ShortText() : "" ) or
      sm.Matches(Description()?Description() : "");
#endif
    return true;
  }

  void RecordingsItemRec::getScraperData(std::string *collectionName) {
    if (m_timeDurationDeviation) m_timeDurationDeviation->start();
    cGetScraperVideo getScraperVideo(NULL, m_recording);
    if (m_timeIdentify) m_timeIdentify->start();
    bool scraper_available = getScraperVideo.call(LiveSetup().GetPluginTvscraper());
    if (m_timeIdentify) m_timeIdentify->stop();
    if (scraper_available) {
      m_scraperVideo.swap(getScraperVideo.m_scraperVideo);
      m_s_videoType = m_scraperVideo->getVideoType();   // sort duplicates
      m_s_dbid = m_scraperVideo->getDbId();             // sort duplicates
      if (m_s_videoType == tSeries || m_s_videoType == tMovie) {
        m_s_episode_number = m_scraperVideo->getEpisodeNumber(); // sort duplicates
        m_s_season_number = m_scraperVideo->getSeasonNumber();   // sort duplicates
        m_language = m_scraperVideo->getLanguage();              // sort duplicates
        m_duration_deviation = m_scraperVideo->getDurationDeviation();  // sort errors
        if (m_timeOverview) m_timeOverview->start();
           // for TV show, we need m_s_title (name of folder)
           // for movie, we need m_s_collection_id
        m_scraperVideo->getOverview(&m_s_title, &m_s_episode_name, &m_s_release_date, &m_s_runtime, &m_s_IMDB_ID, &m_s_collection_id, collectionName);
        if (m_timeOverview) m_timeOverview->stop();
      }
    } else {
// service "GetScraperVideo" is not available
      m_s_videoType = tNone;
    }
    if (m_timeDurationDeviation) m_timeDurationDeviation->stop();
  }
  const cTvMedia &RecordingsItemRec::scraperImage() const {
    if (!m_s_image_requested) {
      m_s_image_requested = true;
      if (m_scraperVideo)
        m_s_image = m_scraperVideo->getImage(m_imageLevels, cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner), false);
      else
        EpgEvents::PosterTvscraper(m_s_image, NULL, m_recording);
    }
    return m_s_image;
  }

  int RecordingsItemRec::CompareTexts(const RecordingsItemRecPtr &second, int *numEqualChars) const
// Compare NameForSearch + ShortText + Description
// if numEqualChars != NULL: return number of equal characters in ShortText + Description
  {
    if (numEqualChars) *numEqualChars = 0;
    int i = NameForSearch().compare(second->NameForSearch() );
    if(i != 0) return i;
// name is identical, compare short text / description
    return CompareStD(second, numEqualChars);
  }

  int RecordingsItemRec::CompareStD(const RecordingsItemRecPtr &second, int *numEqualChars) const
  {
// compare short text & description
    if (numEqualChars) *numEqualChars = 0;
    ShortTextDescription chfirst (        ShortText(),         Description() );
    ShortTextDescription chsecond(second->ShortText(), second->Description() );
    wint_t flc;
    wint_t slc;
    do {
      flc = chfirst.getNextNonPunctChar();
      slc = chsecond.getNextNonPunctChar();
      if ( flc < slc ) return -1;
      if ( flc > slc ) return  1;
      if (numEqualChars) ++(*numEqualChars);
    } while ( flc && slc );
    if (numEqualChars) --(*numEqualChars);
    return 0;
  }

  bool RecordingsItemRec::orderDuplicates(const RecordingsItemRecPtr &second, bool alwaysShortText, bool lang) const {
// this is a < operation. Return false in case of ==   !!!!!
// lang is the last criterion. For sorting, you can always set lang == true
    if (m_s_videoType != second->m_s_videoType) return (int)m_s_videoType < (int)second->m_s_videoType; // 2 if no scraper data. Move these to the end, by using <
    switch (m_s_videoType) {
      case tMovie:
        if (m_s_dbid != second->m_s_dbid) return m_s_dbid < second->m_s_dbid;
        if (!lang) return false;
        return m_language < second->m_language;
      case tSeries:
        if (m_s_dbid != second->m_s_dbid) return m_s_dbid < second->m_s_dbid;
        if (m_s_season_number != second->m_s_season_number) return m_s_season_number < second->m_s_season_number;
        if (m_s_episode_number != second->m_s_episode_number) return m_s_episode_number < second->m_s_episode_number;
        if (m_s_season_number != 0 || m_s_episode_number != 0) {
          if (!lang) return false;  // they are equal => < operator results in false ...
          return m_language < second->m_language;
        }
        if (!lang) return CompareTexts(second) < 0;
        { int cmp = CompareTexts(second);
          if (cmp != 0) return cmp < 0;}
        return m_language < second->m_language;
      default:
// no scraper data available
        int i = NameForSearch().compare(second->NameForSearch() );
        if (i != 0) return i < 0;
        if (!alwaysShortText) return false;
        return CompareStD(second) < 0;
    }
  }

  int RecordingsItemRec::SD_HD() const
  {
     if (m_video_SD_HD >= -1) return m_video_SD_HD; // < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better
     if (m_scraperVideo) m_video_SD_HD = m_scraperVideo->getHD();
     if (m_video_SD_HD >= -1) return m_video_SD_HD;
// see ETSI EN 300 468, V1.15.1 (2016-03) or later, Chapter "6.2.8 Component Descriptor"
     const cComponents *components = RecInfo()->Components();
     bool videoStreamFound = false;
     bool audioStreamFound = false;
     if(components) for( int ix = 0; ix < components->NumComponents(); ix++) {
       tComponent * component = components->Component(ix);
       switch (component->stream) {
       case 1: // MPEG2
       case 5: // H.264
         videoStreamFound = true;
         switch (component->type) {
           case 1:
           case 5: m_video_SD_HD = 0; break; // 4:3
           case 2:
           case 3:
           case 6:
           case 7: m_video_SD_HD = 0; break; // 16:9
           case 4:
           case 8: m_video_SD_HD = 0; break; // >16:9
           case 9:
           case 13: m_video_SD_HD = 1; break; // HD 4:3
           case 10:
           case 11:
           case 14:
           case 15: m_video_SD_HD = 1; break; // HD 16:9
           case 12:
           case 16: m_video_SD_HD = 1; break; // HD >16:9
         }
         break;
       case 9: // HEVC
         // stream_content_ext is missing from VDR info
         // stream_content_ext == 0 -> video
         // stream_content_ext == 1 -> audio
         // we assume stream_content_ext == 0 (video). This might be wrong :( . On the other hand side, all this data might be wrong as broadcasters often ignore this standard
         videoStreamFound = true;
         switch (component->type) {
           case 0:  // stream_content_ext == 0 -> HD. stream_content_ext == 1 -> AC-4 main audio, mono
           case 1:  // stream_content_ext == 0 -> HD. stream_content_ext == 1 -> AC-4 main audio, mono, dialogue enhancement enabled
           case 2:  // stream_content_ext == 0 -> HD. stream_content_ext == 1 -> AC-4 main audio, stereo
           case 3: m_video_SD_HD = 1; break; // HD
           case 4:
           case 5:
           case 6:
           case 7: m_video_SD_HD = 2; break; // UHD
         }
         break;
       case 2: // MPEG2 Audio
       case 4: // AC3 Audio
       case 6: // HE-AAC Audio
       case 7: // reserved for DTS audio modes
         audioStreamFound = true;
         break;
       case 11:
         videoStreamFound = true; // guess as stream_content_ext is missing, assume stream_content_ext = 0xF
         break;
       }
     }
     if(m_video_SD_HD < -1)  // nothing known found
       {
// also check frame rate for radio, as components are not reliable
       if (!videoStreamFound && audioStreamFound && RecInfo()->FramesPerSecond() > 0 && RecInfo()->FramesPerSecond() < 24)
         m_video_SD_HD = -1; // radio
       else
         m_video_SD_HD = 0; // no information -> SD as default
       if(RecInfo()->ChannelName() ) {
         size_t l = strlen(RecInfo()->ChannelName() );
         if( l > 3 && RecInfo()->ChannelName()[l-2] == 'H' && RecInfo()->ChannelName()[l-1] == 'D') m_video_SD_HD = 1;
         }
       }
     return m_video_SD_HD;
  }

void AppendScraperData(cToSvConcat<0> &target, cSv s_IMDB_ID, const cTvMedia &s_image, tvType s_videoType, cSv s_title, int s_season_number, int s_episode_number, cSv s_episode_name, int s_runtime, cSv s_release_date) {
  bool scraperDataAvailable = s_videoType == tMovie || s_videoType == tSeries;
  target.append("\"");
// [2]  IMDB ID
  target.append(s_IMDB_ID);
  target.append("\",\"");
// [3] : image.path  (nach "/tvscraper/")
  target.append(s_image.path);
  target.append("\",\"");
// [4] : "pt" if s_image.width <= s_image.height, otherwise= ""
  if (s_image.width <= s_image.height) target.append("pt");
  target.append("\",\"");
// [5] : title (scraper)
  if (scraperDataAvailable) AppendHtmlEscapedAndCorrectNonUTF8(target, s_title);
  target.append("\",\"");
  if (s_videoType == tSeries && (s_episode_number != 0 || s_season_number != 0)) {
// [6] : season/episode/episode name (scraper)
    target.concat(s_season_number);
    target.concat('E');
    target.concat(s_episode_number);
    target.concat(' ');
    AppendHtmlEscapedAndCorrectNonUTF8(target, s_episode_name);
  }
  target.append("\",\"");
// [7] : runtime (scraper)
  if (scraperDataAvailable && s_runtime) AppendDuration(target, tr("(%d:%02d)"), s_runtime*60);
  target.append("\",\"");
// [8] : release date (scraper)
  if (scraperDataAvailable && !s_release_date.empty() ) target.append(s_release_date);
  target.append("\"");
}

  void RecordingsItemRec::AppendAsJSArray(cToSvConcat<0> &target) const {
    target.append("\"");
// [0] : ID
    target.appendHex(IdHash());
    target.append("\",\"");
// [1] : ArchiveDescr()
    if (IsArchived()) AppendHtmlEscapedAndCorrectNonUTF8(target, ArchiveDescr());
    target.append("\",");
// scraper data
    AppendScraperData(target, m_s_IMDB_ID, scraperImage(), m_s_videoType, m_s_title, m_s_season_number, m_s_episode_number, m_s_episode_name, m_s_runtime, m_s_release_date);
// [9] : Day, time
    target.append(",\"");
    target.appendDateTime(tr("%a,"), StartTime() );  // day of week
    target.concat(' ');
    target.appendDateTime(tr("%b %d %y"), StartTime());  // date
    target.concat(' ');
    target.appendDateTime(tr("%I:%M %p"), StartTime());  // time
    target.append("\", ");
// RecordingErrors, Icon
#if VDRVERSNUM >= 20505
// [10] : Number of recording errors
    target.concat(RecordingErrors() );
#else
    target.append("-100");
#endif
    target.append(", \"");
// [11] HD_SD
    const char *icon_name = nullptr;
#if VDRVERSNUM >= 20605
    if (m_recording && m_recording->Info()) {
      switch (m_recording->Info()->FrameWidth()) {
        case 720:
          icon_name = "720x576";
          break;
        case 1280:
          icon_name = "1280x720";
          break;
        case 1920:
          icon_name = "1920x1080";
          break;
        case 3840:
          icon_name = "3840x2160";
          break;
        default:
          break;
      }
    }
#endif
    if (!icon_name) icon_name = SD_HD() == 0 ? "sd": SD_HD() == 1 ? "hd": SD_HD() >= 2 ? "ud": "rd";
    target.append(icon_name);
    target.append("\", \"");
// [12] channel name
    AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->ChannelName() );
    target.append("\", \"");
// [13] NewR()
    target.append(NewR() );
    target.append("\", \"");
// [14] Name
    AppendQuoteEscapedAndCorrectNonUTF8(target, Name() );
    target.append("\", \"");
// [15] Short text
    const char *text = ShortText();
    if (text && Name() != text && !((Name().substr(0, 1) == "%" && Name().substr(1) == text)) ) AppendHtmlEscapedAndCorrectNonUTF8(target, text);
    target.append("\", \"");
// [16] Description
    AppendTextTruncateOnWord(target, RecInfo()->Description(), LiveSetup().GetMaxTooltipChars(), true);
// [17] recording length deviation
    target.append("\",");
    target.concat(DurationDeviation());
// [18] Path / folder
    target.append(",\"");
    AppendHtmlEscapedAndCorrectNonUTF8(target, (const char *)Recording()->Folder() );
    target.append("\"");
// [19] duration
    target.append(",\"");
    if(Duration() >= 0)
      AppendDuration(target, tr("%d:%02d"), Duration());
    else
      target.append("&nbsp;");
    target.append("\",\"");
// [20] size
    int fileSizeMB = FileSizeMB();
    if(fileSizeMB >= 0)
      if (fileSizeMB >= 1000)
        target.appendFormated(tr("%.1f GB"), (double)fileSizeMB / 1000.);
      else
        target.appendFormated(tr("%'d MB"), fileSizeMB);
    else
      target.append("&nbsp;");
    target.append("\",");
// [21] numTsFiles
    target.concat(NumberTsFiles() );
// [22] frame parameter text
    target.append(",\"");
    StringAppendFrameParams(target, m_recording);
    target.append("\"");
  }

  void RecordingsItemRec::AppendAsJSArray(cToSvConcat<0> &target, std::vector<RecordingsItemRecPtr>::const_iterator recIterFirst, std::vector<RecordingsItemRecPtr>::const_iterator recIterLast, bool &first, cSv filter, bool reverse) {
    if (reverse) {
      for (; recIterFirst != recIterLast;) {
        --recIterLast;
        RecordingsItemRecPtr recItem = *recIterLast;
        if (!recItem->matchesFilter(filter)) continue;
        if (!first) target.append(",");
        else first = false;
        target.concat(recItem->IdI() );
      }
    } else {
      for (; recIterFirst != recIterLast; ++recIterFirst) {
        RecordingsItemRecPtr recItem = *recIterFirst;
        if (!recItem->matchesFilter(filter)) continue;
        if (!first) target.append(",");
        else first = false;
        target.concat(recItem->IdI() );
      }
    }
  }

  /**
   * Implementation of class RecordingsItemDummy
   */
  RecordingsItemDummy::RecordingsItemDummy(const cEvent *event, cScraperVideo *scraperVideo):
    RecordingsItemRec(event->Title() ),
    m_event(event)
    {
      if (scraperVideo) {
        m_s_videoType = scraperVideo->getVideoType();
        m_s_dbid = scraperVideo->getDbId();
        m_s_episode_number = scraperVideo->getEpisodeNumber();
        m_s_season_number = scraperVideo->getSeasonNumber();
        m_language = scraperVideo->getLanguage();
        m_video_SD_HD = scraperVideo->getHD();
      } else {
        m_s_videoType = tNone;
      }
    }

  bool operator< (const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b) { return *a < b; }
  bool operator< (cSv a, const RecordingsItemDirPtr &b) { return a < b->Name(); }
  bool operator< (const RecordingsItemDirPtr &a, cSv b) { return a->Name() < b; }
  bool operator< (int a, const RecordingsItemDirPtr &b) { return *b > a; }
  bool operator< (const RecordingsItemDirPtr &a, int b) { return *a < b; }

  /**
   *  Implementation of class RecordingsTree:
   */
  RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
    m_maxLevel(0),
    m_root(std::make_shared<RecordingsItemDir>("", 0))
  {
//   esyslog("live: DH: ****** RecordingsTree::RecordingsTree() ********");
   cMeasureTime timeRecs, timeIdentify, timeOverview, timeImage, timeDurationDeviation, timeNumTsFiles, timeItemRec;

// check availability of scraper data
    m_creation_timestamp = time(0);
    cGetScraperVideo getScraperVideo;
    bool scraperDataAvailable = getScraperVideo.call(LiveSetup().GetPluginTvscraper());
    RecordingsItemDirPtr recPtrTvShows = std::make_shared<RecordingsItemDir>(tr("TV shows"), 1);
    RecordingsItemDirPtr recPtrMovieCollections = std::make_shared<RecordingsItemDir>(tr("Movie collections"), 1);
// create "base" folders
    m_allRecordings.clear();
    if (scraperDataAvailable) {
      m_root->m_cmp_dir = RecordingsItemPtrCompare::BySeason;
      recPtrTvShows->m_s_season_number = 1;
      m_root->m_subdirs.push_back(recPtrTvShows);
      recPtrMovieCollections->m_s_season_number = 2;
      m_root->m_subdirs.push_back(recPtrMovieCollections);
      RecordingsItemDirPtr recPtrOthers = std::make_shared<RecordingsItemDir>(tr("File system view"), 1);
      recPtrOthers->m_s_season_number = 3;
      m_rootFileSystem = recPtrOthers;
      m_root->m_subdirs.push_back(recPtrOthers);
    } else {
      m_rootFileSystem = m_root;
    }
// add all recordings
    LOCK_RECORDINGS_READ;
    for (cRecording* recording = (cRecording *)Recordings->First(); recording; recording = (cRecording *)Recordings->Next(recording)) {
      if (scraperDataAvailable) m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() + 1);
      else m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() );

      RecordingsItemDirPtr dir = m_rootFileSystem;
      cSv name(recording->Name());

      size_t index = 0;
      size_t pos = 0;
      do {
        pos = name.find('~', index);
        if (pos != std::string::npos) {
// note: the dir returned is the added (or existing) subdir named name.substr(index, pos - index)
          dir = dir->addDirIfNotExists(name.substr(index, pos - index) );
          index = pos + 1;
          // esyslog("live: DH: current dir: '%s'", dir->Name().c_str());
        }
        else {
          cSv recName = name.substr(index, name.length() - index);
          timeRecs.start();
          RecordingsItemRecPtr recPtr = std::make_shared<RecordingsItemRec>(recName, recording, &timeIdentify, &timeOverview, &timeImage, &timeDurationDeviation, &timeNumTsFiles, &timeItemRec);
          timeRecs.stop();
          dir->m_entries.push_back(recPtr);
          m_allRecordings.push_back(recPtr);
          // esyslog("live: DH: added rec: '%.*s'", (int)recName.length(), recName.data());
          if (scraperDataAvailable) {
            switch (recPtr->scraperVideoType() ) {
              case tMovie:
                if (recPtr->m_s_collection_id == 0) break;
                dir = recPtrMovieCollections->addDirCollectionIfNotExists(recPtr->m_s_collection_id, recPtr);
                dir->m_entries.push_back(recPtr);
                break;
              case tSeries:
                dir = recPtrTvShows->addDirIfNotExists(recPtr->scraperName() );
                dir->setTvShow(recPtr);
                if (recPtr->scraperSeasonNumber() != 0 || recPtr->scraperEpisodeNumber() != 0) {
                  dir = dir->addDirSeasonIfNotExists(recPtr->scraperSeasonNumber(), recPtr);
                }
                dir->m_entries.push_back(recPtr);
                break;
              default:  // do nothing
                break;
            }
          }
        }
      } while (pos != std::string::npos);
    }
    if (scraperDataAvailable) {
      for (auto it = recPtrTvShows->m_subdirs.begin(); it != recPtrTvShows->m_subdirs.end(); ) {
        if ((*it)->numberOfRecordings() < 2) it = recPtrTvShows->m_subdirs.erase(it);
        else ++it;
      }
      for (auto it = recPtrMovieCollections->m_subdirs.begin(); it != recPtrMovieCollections->m_subdirs.end(); ) {
        if ((*it)->numberOfRecordings() < 2) it = recPtrMovieCollections->m_subdirs.erase(it);
        else ++it;
      }
    }
    m_root->finishRecordingsTree();
/*
                 timeRecs.print("live: timeRecs  ");
              timeItemRec.print("live: ItemRec   ");
             timeIdentify.print("live: Identify  ");
             timeOverview.print("live: Overview  ");
                timeImage.print("live: Image     ");
    timeDurationDeviation.print("live: Scraper   ");
           timeNumTsFiles.print("live: NumTsFiles");
*/
  }

  RecordingsTree::~RecordingsTree()
  {
    // esyslog("live: DH: ****** RecordingsTree::~RecordingsTree() ********");
  }
  const std::vector<RecordingsItemRecPtr> *RecordingsTree::allRecordings(eSortOrder sortOrder) {
    if (sortOrder == m_sortOrder) return &m_allRecordings_other_sort;
    if (sortOrder == eSortOrder::name) {
      if (!m_allRecordingsSorted) {
        std::sort(m_allRecordings.begin(), m_allRecordings.end(), RecordingsItemPtrCompare::getComp(eSortOrder::name));
        m_allRecordingsSorted = true;
      }
      return &m_allRecordings;
    }
    if (m_allRecordings_other_sort.empty() ) m_allRecordings_other_sort = m_allRecordings;
    std::sort(m_allRecordings_other_sort.begin(), m_allRecordings_other_sort.end(), RecordingsItemPtrCompare::getComp(sortOrder));
    m_sortOrder = sortOrder;
    return &m_allRecordings_other_sort;
  }

  /**
   *  Implementation of class RecordingsTreePtr:
   */
  RecordingsTreePtr::RecordingsTreePtr() :
    std::shared_ptr<RecordingsTree>(),
    m_recManPtr()
  {
  }

  RecordingsTreePtr::RecordingsTreePtr(RecordingsManagerPtr recManPtr, std::shared_ptr<RecordingsTree> recTree) :
    std::shared_ptr<RecordingsTree>(recTree),
    m_recManPtr(recManPtr)
  {
  }

  RecordingsTreePtr::~RecordingsTreePtr()
  {
  }

  /**
   *  Implementation of function LiveRecordingsManager:
   */
  RecordingsManagerPtr LiveRecordingsManager()
  {
    RecordingsManagerPtr r = RecordingsManager::m_recMan.lock();
    if (r) {
      return r;
    }
    else {
      RecordingsManagerPtr n(new RecordingsManager);
      RecordingsManager::m_recMan = n;
      return n;
    }
  }

// icon with recording errors and tooltip
std::string recordingErrorsHtml(int recordingErrors) {
#if VDRVERSNUM >= 20505
  std::string result;
  result.append("<div class=\"recording_errors\"><img src=\"");

  if (recordingErrors == 0) {
    result.append(LiveSetup().GetThemedLink("img", "NoRecordingErrors.png"));
    result.append("\" title=\"");
    result.append(tr("No recording errors"));
  }
  if (recordingErrors == -1) {
    result.append(LiveSetup().GetThemedLink("img", "NotCheckedForRecordingErrors.png"));
    result.append("\" title=\"");
    result.append(tr("Recording errors unknown"));
  }
  if (recordingErrors >   0) {
    result.append(LiveSetup().GetThemedLink("img", "RecordingErrors.png"));
    result.append("\" title=\"");
    result.append(tr("Number of recording errors:"));
    result.append(" ");
    result.append(cToSvInt(recordingErrors));
  }
  result.append("\" width = \"16px\"/> </div>");
  return result;
#else
  return std::string();
#endif
}

// find duplicates
bool ByScraperDataAvailable(const RecordingsItemRecPtr &first, tvType videoType) {
  return (int)first->scraperVideoType() < (int)videoType; // tNone if no scraper data. Move these to the end, by using <
}

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where NO scraper data are available.
  const std::vector<RecordingsItemRecPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
// sorting for duplicatesLanguage is OK, even if language is not required here (language is ignored later)
  std::vector<RecordingsItemRecPtr>::const_iterator currentRecItem, recIterUpName, recIterUpShortText, recIterLowShortText;
  bool isSeries;

// see https://www.fluentcpp.com/2017/08/01/overloaded-functions-stl/
// static_cast<bool(*)(const RecordingsItemPtr &first, int videoType)>(f));
// #define RETURNS(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__){ return __VA_ARGS__; }
// #define LIFT(f) [](auto&&... xs) RETURNS(f(::std::forward<decltype(xs)>(xs)...))
// std::for_each(begin(numbers), end(numbers), LIFT(f));
  for (currentRecItem = std::lower_bound(recItems->begin(), recItems->end(), tNone, ByScraperDataAvailable);
        currentRecItem != recItems->end(); ) {
    recIterUpName  = std::upper_bound (currentRecItem , recItems->end(), *currentRecItem, RecordingsItemPtrCompare::ByDuplicatesName);
    int numberOfRecordingsWithThisName = std::distance(currentRecItem, recIterUpName);

    if (numberOfRecordingsWithThisName < 2) { currentRecItem++; continue; } // there is only one recording with this name
    if (numberOfRecordingsWithThisName > 5) isSeries = true; else isSeries = false;
    if (isSeries) {
      for (; currentRecItem != recIterUpName;){
        recIterLowShortText = currentRecItem;
        recIterUpShortText  = std::upper_bound (currentRecItem , recIterUpName, *currentRecItem, RecordingsItemPtrCompare::ByDuplicates);
        currentRecItem++;
        if (currentRecItem == recIterUpShortText ) continue; // there is only one recording with this short text
        for(currentRecItem = recIterLowShortText; currentRecItem != recIterUpShortText; currentRecItem++)
          DuplicateRecItems.push_back(*currentRecItem);
      }
    } else { // not a series
      for(; currentRecItem != recIterUpName; currentRecItem++)
        DuplicateRecItems.push_back(*currentRecItem);
    }
  }
}

void addDuplicateRecordingsLang(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree) {
// add duplicate recordings where scraper data are available.
// add only recordings with different language
  const std::vector<RecordingsItemRecPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemRecPtr>::const_iterator currentRecItem, nextRecItem, recIterUpName, recIterLowName;

  for (currentRecItem = recItems->begin(); currentRecItem != recItems->end(); currentRecItem++) {
    if ( !(*currentRecItem)->scraperDataAvailable() ) break;
    recIterLowName = currentRecItem;
    recIterUpName  = std::upper_bound (currentRecItem , recItems->end(), *currentRecItem, RecordingsItemPtrCompare::ByDuplicatesName);

    if (recIterLowName == recIterUpName ) continue; // there is no recording with this name (internal error)
    nextRecItem = recIterLowName;
    nextRecItem++;
    if (nextRecItem == recIterUpName ) continue; // there is only one recording with this name
    for(currentRecItem = recIterLowName; currentRecItem != recIterUpName && nextRecItem != recIterUpName; currentRecItem++, nextRecItem++)
      if (RecordingsItemPtrCompare::ByDuplicatesLanguage(*currentRecItem, *nextRecItem) ) {
        DuplicateRecItems.push_back(*currentRecItem);
        DuplicateRecItems.push_back(*nextRecItem);
      }
  }
}
void addDuplicateRecordingsSd(std::vector<RecordingsItemRecPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where scraper data are available.
// recordings with different languages are NOT duplicates
  const std::vector<RecordingsItemRecPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemRecPtr>::const_iterator currentRecItem, recIterUpName, recIterLowName;

  for (currentRecItem = recItems->begin(); currentRecItem != recItems->end(); ) {
    if ( !(*currentRecItem)->scraperDataAvailable() ) break;
    recIterLowName = currentRecItem;
    recIterUpName  = std::upper_bound (currentRecItem , recItems->end(), *currentRecItem, RecordingsItemPtrCompare::ByDuplicatesLanguage);

    currentRecItem++;
    if (recIterLowName == recIterUpName ) continue; // there is no recording with this name (internal error)
    if (currentRecItem == recIterUpName ) continue; // there is only one recording with this name
    for(currentRecItem = recIterLowName; currentRecItem != recIterUpName; currentRecItem++)
      DuplicateRecItems.push_back(*currentRecItem);
  }
}


} // namespace vdrlive
