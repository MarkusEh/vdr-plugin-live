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
#include <vdr/menu.h>

#define TIMERRECFILE      "/.timer"

namespace vdrlive {

  /**
   *  Implementation of class RecordingsManager:
   */
  void RecordingsManager::setSortOrder(eSortOrder sortOrder, bool backwards, cSv filter) {
    EnsureValidData();
    all_recordings_sorted = all_recordings[(int)(sortOrder)];
    if (all_recordings_sorted.empty() ) {
      all_recordings_sorted.assign(all_recordings_iterator(), all_recordings_iterator(iterator_end() ));
      std::sort(all_recordings_sorted.begin(), all_recordings_sorted.end(), RecordingsItemPtrCompare::getComp(sortOrder));
    }
    m_backwards = backwards;
    if (filter.empty() )
      m_filter = false;
    else {
      m_filter_regex = getRegex(filter, g_locale, std::regex_constants::icase |
                                                  std::regex_constants::nosubs |
                                                  std::regex_constants::collate);
      m_filter = true;
    }
  }

  std::shared_ptr<RecordingsTree> RecordingsManager::m_recTree;

/*
 cStateKey RecordingsManager::m_recordingsStateKey, together with
 time_t RecordingsManager::m_last_recordings_update = 0
 are used to check for any relevant changes (in VDRs recordings or
 in scraper data)

RecordingsTreePtr RecordingsManager::GetRecordingsTree()
is checking for such changes. If there are none, it returns a cached
recording tree (m_recTree)
Otherwise, the rec tree is re-created from currnet data.
*/

  RecordingsTreePtr RecordingsManager::GetRecordingsTree()
  {
    EnsureValidData();
    return RecordingsTreePtr(m_recTree);
  }

  const cRecording *RecordingsManager::GetByHash(cSv hash, const cRecordings* Recordings)
  {
    if (hash.length() != 42) return nullptr;
    if (hash.compare(0, 10, "recording_") != 0) return nullptr;
    XXH128_hash_t xxh = parse_hex_128(hash.substr(10));
    for (const cRecording* rec = Recordings->First(); rec; rec = Recordings->Next(rec)) {
      if (XXH128_isEqual(XXH3_128bits(rec->FileName(), strlen(rec->FileName())), xxh)) return rec;
    }
    return nullptr;
  }

  RecordingsItemRec* const RecordingsManager::GetByIdHash(cSv hash)
  {
    if (hash.length() != 42) return 0;
    if (hash.compare(0, 10, "recording_") != 0) return 0;
    XXH128_hash_t xxh = parse_hex_128(hash.substr(10));
    for (RecordingsItemRec * recItem : *GetRecordingsTree()->allRecordings()) {
      if (XXH128_isEqual(recItem->IdHash(), xxh)) return recItem;
    }
    return nullptr;
  }

  bool RecordingsManager::UpdateRecording(cSv hash, cSv directory, cSv name, bool copy, cSv title, cSv shorttext, cSv description)
  {
    std::string new_filename = FileSystemExchangeChars(directory.empty() ? name : cSv(cToSvReplace(directory, "/", "~") << "~" << name), true);
    // Check for injections that try to escape from the video dir.
    if (new_filename.compare(0, 3, "..~") == 0 || new_filename.find("~..") != std::string::npos) {
      esyslog("live: renaming failed: new name invalid \"%.*s\"", (int)new_filename.length(), new_filename.data());
      return false;
    }

    LOCK_RECORDINGS_WRITE;
    cRecording *recording = const_cast<cRecording *>(RecordingsManager::GetByHash(hash, Recordings));
    if (!recording) return false;

    std::string oldname = recording->FileName();
    size_t found = oldname.find_last_of("/");
    if (found == std::string::npos) return false;

    std::string newname = concat(cVideoDirectory::Name(), "/", new_filename, cSv(oldname).substr(found));

    if (oldname != newname) {
      if (recording->IsInUse() ) return false;
      if (!MoveDirectory(oldname, newname, copy)) {
        esyslog("live: renaming failed from '%.*s' to '%s'", (int)oldname.length(), oldname.data(), newname.c_str());
        return false;
      }

      if (!copy)
        Recordings->DelByName(oldname.c_str());
      Recordings->AddByName(newname.c_str());
      recording = Recordings->GetByName(newname.c_str());   // old pointer to recording invalid after DelByName/AddByName
      if (copy)
        cRecordingUserCommand::InvokeCommand(RUC_COPIEDRECORDING, newname.c_str(), oldname.c_str());
      else {
        if (strcmp(strgetbefore(oldname.c_str(), '/', 2), strgetbefore(newname.c_str(), '/', 2)))
          cRecordingUserCommand::InvokeCommand(RUC_MOVEDRECORDING, newname.c_str(), oldname.c_str());
        else
          cRecordingUserCommand::InvokeCommand(RUC_RENAMEDRECORDING, newname.c_str(), oldname.c_str());
      }
    }

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
    Recordings->TouchUpdate();
    return true;
  }

  void RecordingsManager::DeleteResume(cRecording const * recording)
  {
    if (!recording)
      return;

    cResumeFile ResumeFile(recording->FileName(), recording->IsPesRecording());
    ResumeFile.Delete();
  }

  void RecordingsManager::DeleteMarks(cRecording const * recording)
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

  int RecordingsManager::DeleteRecording(cSv recording_hash, std::string *name)
  {
    LOCK_RECORDINGS_WRITE;
    cRecording *recording = const_cast<cRecording *>(RecordingsManager::GetByHash(recording_hash, Recordings));
    if (!recording) return 1;
    if (name) *name = cSv(recording->Name());

    if (recording->IsInUse() ) return 2;

    std::string l_name(recording->FileName());
    bool result = recording->Delete();
    Recordings->DelByName(l_name.c_str());
    return result?0:3;
  }

  int RecordingsManager::GetArchiveType(cRecording const * recording)
  {
// 1: on DVD
// 2: on HDD
// 0: "normal" VDR recording
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

      /**
       *  Is this recording currently played?
       *  Return codes:
       *    0: this recording is currently played
       *    1: this recording does not exist
       *    2: no recording is played
       *    3: another recording is played
       */
  int RecordingsManager::CheckReplay(cSv recording_hash, std::string *fileName) {
    LOCK_RECORDINGS_READ;
    const cRecording *recording = GetByHash(recording_hash, Recordings);
    if (!recording)  return 1;
    if (fileName) *fileName = cSv(recording->FileName());
    const char *current = cReplayControl::NowReplaying();
    if (!current) return 2;
    if (0 != strcmp(current, recording->FileName())) return 3;
    return 0;
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

  void update_all_recordings();
  void update_all_recordings_scraper();
  void RecordingsManager::EnsureValidData()
// ensure m_recTree is up to date
  {
    if (m_last_recordings_update + 1 > time(NULL) ) return; // don't update too often

    // StateChanged must be executed every time, so not part of
    // the short cut evaluation in the if statement below.
    bool stateChanged = StateChanged();
// check: changes on scraper data?
    bool scraperChanged = false;
    cGetScraperUpdateTimes scraperUpdateTimes;
    if (scraperUpdateTimes.call(LiveSetup().GetPluginTvscraper()) )
      scraperChanged = scraperUpdateTimes.m_recordingsUpdateTime > m_last_recordings_update;

    if (stateChanged || all_recordings[(int)(RecordingsManager::eSortOrder::id)].empty() ) update_all_recordings();
    if (scraperChanged) update_all_recordings_scraper();

    if (stateChanged || scraperChanged || (!m_recTree) ) {
      m_last_recordings_update = time(NULL);
      std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
      m_recTree = std::shared_ptr<RecordingsTree>(new RecordingsTree());
      std::chrono::duration<double> timeNeeded = std::chrono::high_resolution_clock::now() - begin;
      dsyslog("live: DH: ------ RecordingsTree::RecordingsTree() --------, required time: %9.5f", timeNeeded.count() );
    }
  }

  /**
   * Implementation of class RecordingsItemPtrCompare
   */

  int RecordingsItemPtrCompare::FindBestMatch(RecordingsItemRec *&BestMatch, const std::vector<RecordingsItemRec*>::const_iterator & First, const std::vector<RecordingsItemRec*>::const_iterator & Last, const RecordingsItemRec * EPG_Entry) {
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
   std::vector<RecordingsItemRec*>::const_iterator bestMatchIter;
   for ( std::vector<RecordingsItemRec*>::const_iterator iter = First; iter != Last; ++iter)
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

  bool RecordingsItemPtrCompare::ByAscendingDate(const RecordingsItemRec * first, const RecordingsItemRec * second)
  {
    return (first->StartTime() < second->StartTime());
  }

  bool RecordingsItemPtrCompare::ByDuplicatesName(const RecordingsItemRec * first, const RecordingsItemRec * second)  // return first < second
  {
           return first->orderDuplicates(second, false);
  }

  bool RecordingsItemPtrCompare::ByDuplicates(const RecordingsItemRec * first, const RecordingsItemRec * second)  // return first < second
  {
          return first->orderDuplicates(second, true);
  }

  bool RecordingsItemPtrCompare::ByDuplicatesLanguage(const RecordingsItemRec * first, const RecordingsItemRec * second)
  {
          return first->orderDuplicates(second, true, true);
  }

  const char *firstNonPunct(const char *s) {
// returns first non-punct char in s
    while (ispunct(*s)) ++s;
    return s;
  }
  int compareWithLocale(cStr first, cStr second) {
    const char *ls = firstNonPunct(first.c_str());
    const char *rs = firstNonPunct(second.c_str());
    return strcoll(ls, rs);
// see https://en.cppreference.com/w/cpp/string/byte/strcoll
// see https://en.cppreference.com/w/cpp/locale/collate/compare
// Compares the character sequence [low1, high1) to the character sequence [low2, high2)
// we use the c-version, as it is usually much faster
  }
  bool RecordingsItemPtrCompare::ByAscendingNameDescSort(const RecordingsItemRec * first, const RecordingsItemRec * second)  // return first < second
// used for sort
  {
    int i = compareWithLocale(first->NameStr(), second->NameStr() );
    if(i != 0) return i < 0;
    return first->CompareStD(second) < 0;
  }
  bool RecordingsItemPtrCompare::ByAscendingNameSort(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second)  // return first < second
  {
    return compareWithLocale(first->NameStr(), second->NameStr()) < 0;
  }

  bool RecordingsItemPtrCompare::ByDescendingRecordingErrors(const RecordingsItemRec * first, const RecordingsItemRec * second) {
     return first->RecordingErrors() > second->RecordingErrors();
  }
  bool RecordingsItemPtrCompare::ByDescendingDurationDeviation(const RecordingsItemRec * first, const RecordingsItemRec * second) {
    return first->DurationDeviation() > second->DurationDeviation();
  }

  bool RecordingsItemPtrCompare::ByEpisode(const RecordingsItemRec * first, const RecordingsItemRec * second) {
    return first->scraperEpisodeNumber() < second->scraperEpisodeNumber();
  }

  bool RecordingsItemPtrCompare::BySeason(const RecordingsItemDirPtr & first, const RecordingsItemDirPtr & second) {
    return first->scraperSeasonNumber() < second->scraperSeasonNumber();
  }

  bool RecordingsItemPtrCompare::ByReleaseDate(const RecordingsItemRec * first, const RecordingsItemRec * second) {
    return first->scraperReleaseDate() < second->scraperReleaseDate();
  }

  tCompRec RecordingsItemPtrCompare::getComp(RecordingsManager::eSortOrder sortOrder) {
    switch (sortOrder) {
      case RecordingsManager::eSortOrder::name: return &RecordingsItemPtrCompare::ByAscendingNameDescSort;
      case RecordingsManager::eSortOrder::date: return &RecordingsItemPtrCompare::ByAscendingDate;
      case RecordingsManager::eSortOrder::errors: return &RecordingsItemPtrCompare::ByDescendingRecordingErrors;
      case RecordingsManager::eSortOrder::durationDeviation: return &RecordingsItemPtrCompare::ByDescendingDurationDeviation;
      case RecordingsManager::eSortOrder::duplicatesLanguage: return &RecordingsItemPtrCompare::ByDuplicatesLanguage;
      default:
        esyslog("live: ERROR, RecordingsItemPtrCompare::getComp, sortOrder %d unknown", (int)sortOrder);
        return &RecordingsItemPtrCompare::ByAscendingNameDescSort;
    }
  }

bool searchNameDesc(RecordingsItemRec *&RecItem, const std::vector<RecordingsItemRec*> *RecItems, const cEvent *event, cScraperVideo *scraperVideo) {
  if (RecItems->empty() ) return false;  // there are no recordings

// find all recordings with equal name
  RecordingsItemRec dummy_o(event, scraperVideo);
  RecordingsItemRec* dummy = &dummy_o;
  dummy->finalize();
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

  RecordingsItemDirPtr RecordingsItemDir::addDirCollectionIfNotExists(int collectionId, const RecordingsItemRec *rPtr) {
    std::vector<RecordingsItemDirPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), collectionId);
    if (iter != m_subdirs.end() && !(collectionId < *iter) ) return *iter;
    RecordingsItemDirPtr dirPtr2 = std::make_shared<RecordingsItemDirCollection>(Level() + 1, rPtr);
    m_subdirs.insert(iter, dirPtr2);
    return dirPtr2;
  }

  RecordingsItemDirPtr RecordingsItemDir::addDirSeasonIfNotExists(int seasonNumber, const RecordingsItemRec *rPtr) {
    std::vector<RecordingsItemDirPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), seasonNumber);
    if (iter != m_subdirs.end() && !(seasonNumber < *iter) ) return *iter;
    RecordingsItemDirPtr dirPtr2 = std::make_shared<RecordingsItemDirSeason>(Level() + 1, rPtr);
    m_subdirs.insert(iter, dirPtr2);
    return dirPtr2;
  }

  const std::vector<RecordingsItemRec*> *RecordingsItemDir::getRecordings(RecordingsManager::eSortOrder sortOrder)
  {
    if (m_cmp_rec) return &m_entries;
    if (sortOrder == RecordingsManager::eSortOrder::name) {
      if (!m_entriesSorted) {
        std::sort(m_entries.begin(), m_entries.end(), RecordingsItemPtrCompare::getComp(RecordingsManager::eSortOrder::name));
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
  void RecordingsItemDir::set_rec_iterator(const_rec_iterator<RecordingsItemRec> &rec_it) {
    rec_it.set_container(m_entries);
  }

  bool RecordingsItemDir::checkNew() const{
    for (const auto &rec:m_entries) if (rec->IsNew()) return true;
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

  void RecordingsItemDir::setTvShow(const RecordingsItemRec *rPtr) {
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
    utf8_sanitize_string(m_s_image.path);
    return m_s_image;
  }

  bool RecordingsItemDir::matchesFilter(cSv filter) const {
    if (filter.empty()) return true;
    for (const auto &rec:m_entries) if (rec->matchesFilter(filter)) return true;
    for (const auto &subdir:m_subdirs) if (subdir->matchesFilter(filter)) return true;
    return false;
  }
  bool RecordingsItemDir::matchesFilter(std::regex *regex_filter) const {
    if (!regex_filter) return true;
    for (const auto &rec:m_entries) if (rec->matches_regex(regex_filter)) return true;
    for (const auto &subdir:m_subdirs) if (subdir->matchesFilter(regex_filter)) return true;
    return false;
  }


  /**
   *  Implementation of class RecordingsItemDirCollection:
   */
  RecordingsItemDirCollection::RecordingsItemDirCollection(int level, const RecordingsItemRec *rPtr):
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
  RecordingsItemDirSeason::RecordingsItemDirSeason(int level, const RecordingsItemRec *rPtr):
    RecordingsItemDir(cToSvInt(rPtr->m_s_season_number), level)
  {
    m_cmp_rec = RecordingsItemPtrCompare::ByEpisode;
//    m_imageLevels = cImageLevels(eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection);
    m_imageLevels = cImageLevels(eImageLevel::seasonMovie);
    m_rec_item = rPtr;
    m_s_season_number = m_rec_item->m_s_season_number;
  }
  RecordingsItemDirSeason::~RecordingsItemDirSeason() { }

int GetNumberOfTsFiles(cSv fileName) {
// find our number of ts files
  size_t folder_length = fileName.length();
  cToSvConcat file_path(fileName, "/00001.ts");
  struct stat buffer;
  uint32_t num_ts_files;
  std::chrono::time_point<std::chrono::high_resolution_clock> timeStart = std::chrono::high_resolution_clock::now();
  for (num_ts_files = 1; num_ts_files < 100000u; ++num_ts_files) {
    file_path.erase(folder_length+1);
    file_path.appendInt<5>(num_ts_files).append(".ts");
// stat is 10% faster than access on my system. On others, there is a larger difference
// see https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
    if (stat(file_path.c_str(), &buffer) != 0) break;
  }
  std::chrono::duration<double> timeNeeded = std::chrono::high_resolution_clock::now() - timeStart;
  if (timeNeeded.count() > 0.1)
    dsyslog("live, time GetNumberOfTsFiles: %f, recording %.*s, num ts files %d", timeNeeded.count(), (int)fileName.length(), fileName.data(), num_ts_files - 1);
  return num_ts_files - 1;
}

bool StillRecording(cSv Directory) {
  struct stat buffer;
  return stat(cToSvConcat(Directory, TIMERRECFILE).c_str(), &buffer) == 0;
}
  /**
   *  Implementation of class RecordingsItemRec:
   */
// check recording != nullpointer before calling!
  RecordingsItemRec::RecordingsItemRec(const cRecording* recording, cMeasureTime *timeIdentify, cMeasureTime *timeOverview, cMeasureTime *timeDurationDeviation, cMeasureTime *timeItemRec):
    m_id(recording->Id()),
    m_hash(recording->FileName()?XXH3_128bits(recording->FileName(), strlen(recording->FileName()) ):XXH3_128bits("no file", 7)),
    m_name_vdr(cSv(recording->Name())),
    m_file_name(cSv(recording->FileName()))
  {
    m_IsNew = recording->IsNew();
    m_startTime = recording->Start();
    if (!StillRecording(m_file_name)) {
      m_fileSizeMB = DirSizeMB(m_file_name.c_str());
      m_duration = recording->LengthInSeconds();
      m_number_ts_files = GetNumberOfTsFiles(m_file_name);
#if VDRVERSNUM >= 20505
      if (recording->Info()) m_recordingErrors = recording->Info()->Errors();
#endif
    }
#if VDRVERSNUM < 20505
    m_recordingErrors = 0;
#endif
    const cRecordingInfo *info = recording->Info();
    if (info) {
      m_shortText = cSv(info->ShortText());
      m_description = cSv(info->Description());
      m_channelName = cSv(info->ChannelName());
#if VDRVERSNUM >= 20605
      m_framesPerSecond  = info->FramesPerSecond();
      m_frameWidth = info->FrameWidth();
      m_frameHeight = info->FrameHeight();
      m_scanType = info->ScanType();
      m_aspectRatio = info->AspectRatio();
#endif
    }
// scraper data
    if (timeItemRec) timeItemRec->start();
    m_timeIdentify = timeIdentify;
    m_timeOverview = timeOverview;
    m_timeDurationDeviation = timeDurationDeviation;

    getScraperData(recording);
    if (timeItemRec) timeItemRec->stop();
    m_video_SD_HD = get_SD_HD(info);  // we might use scraper data for this
  }
  bool RecordingsItemRec::has_changed(const cRecording* recording) {
    if (!XXH128_isEqual(m_hash, recording->FileName()?XXH3_128bits(recording->FileName(), strlen(recording->FileName()) ):XXH3_128bits("no file", 7))) return true;
    if (!is_equal_utf8_sanitized_string(m_name_vdr, recording->Name() )) return true;
    m_IsNew = recording->IsNew();
    if (m_startTime != recording->Start()) return true;
    const cRecordingInfo *info = recording->Info();
    if (info) {
      if (!is_equal_utf8_sanitized_string(m_shortText, info->ShortText())) return true;
      if (!is_equal_utf8_sanitized_string(m_description, info->Description())) return true;
      if (!is_equal_utf8_sanitized_string(m_channelName, info->ChannelName())) return true;
#if VDRVERSNUM >= 20605
      if (m_framesPerSecond != info->FramesPerSecond()) return true;
      if (m_frameWidth != info->FrameWidth()) return true;
      if (m_frameHeight != info->FrameHeight()) return true;
      if (m_scanType != info->ScanType()) return true;
      if (m_aspectRatio != info->AspectRatio()) return true;
#endif
    } else {
      if (!m_shortText.empty() || !m_description.empty() || !m_channelName.empty()) return true;
    }
    m_seen = true;
    return false;
  }

  RecordingsItemRec::RecordingsItemRec(const cEvent *event, cScraperVideo *scraperVideo):
    m_id(-1),
    m_hash(XXH3_128bits("dummy recording", 15)),
    m_name_vdr(cSv(event->Title()))
  {
    m_shortText = std::string(cSv(event->ShortText()));
    m_description = std::string(cSv(event->Description()));
    m_startTime = event->StartTime();
    m_duration = event->Duration() / 60; // duration in minutes

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

  void RecordingsItemRec::finalize() {
// everything that can be doen without needing the vdr cRecording object
// utf8 sanitize all strings
    utf8_sanitize_string(m_name_vdr);
    utf8_sanitize_string(m_shortText);
    utf8_sanitize_string(m_description);
    utf8_sanitize_string(m_channelName);
    utf8_sanitize_string(m_s_title);
    utf8_sanitize_string(m_s_episode_name);
    utf8_sanitize_string(m_s_IMDB_ID);
    utf8_sanitize_string(m_s_release_date);
// get m_name, m_name_str, m_folder
    const_split_iterator name_split(iterator_end(), m_name_vdr, '~');
    --name_split;
    m_name = *name_split;
    m_name_str = name_split.pos();
    if (name_split != iterator_begin()) {
//      int len = std::distance((const char *)m_name_vdr.data(), name_split.pos());
//      if (len < 2) esyslog("live, ERROR in finalize, len =%d, m_name_str = %s", len, m_name_vdr.c_str());
      m_folder = cSv(m_name_vdr.data(), std::distance((const char *)m_name_vdr.data(), name_split.pos())-1);
    }
    m_seen = true;
  }


  int RecordingsItemRec::FileSizeMB() const {
    if (m_fileSizeMB < 0) {
       int fs = DirSizeMB(m_file_name.c_str());
       if (StillRecording(m_file_name))
          return fs; // check again later for ongoing recordings
       m_fileSizeMB = fs;
       }
    return m_fileSizeMB;
  }
  int RecordingsItemRec::NumberTsFiles() const {
    if (m_number_ts_files < 0) {
       int number_ts_files = GetNumberOfTsFiles(m_file_name);
       if (StillRecording(m_file_name))
          return number_ts_files; // check again later for ongoing recordings
       m_number_ts_files = number_ts_files;
       }
    return m_number_ts_files;
  }
  int RecordingsItemRec::Duration() const {
    if (m_duration < 0) {
      LOCK_RECORDINGS_READ;
      const cRecording* recording = Recordings->GetById(m_id);
      if (recording) {
        int duration = recording->LengthInSeconds();
        if (StillRecording(m_file_name)) return duration;
        m_duration = duration;
      }
    }
    return m_duration;
  }
  int RecordingsItemRec::RecordingErrors() const {
#if VDRVERSNUM >= 20505
    if (m_recordingErrors < 0) {
      LOCK_RECORDINGS_READ;
      const cRecording* recording = Recordings->GetById(m_id);
      if (recording && recording->Info() ) {
        int recordingErrors = recording->Info()->Errors();
        if (StillRecording(m_file_name)) return recordingErrors;
        m_recordingErrors = recordingErrors;
      }
    }
    return m_recordingErrors;
#else
    return 0;
#endif
  }


  void update_all_recordings () {
    cMeasureTime time_update_all_recordings;
    time_update_all_recordings.start();
    bool change = false;
// mark all recordings in cache as not seen
    for (RecordingsItemRec* r: all_recordings_iterator(iterator_begin() )) r->set_seen(false);
    {
// increase vector with all_recordings[(int)(RecordingsManager::eSortOrder::id)] to the required size
      int max_id = 0;
      LOCK_RECORDINGS_READ;
      for (const cRecording* recording = Recordings->First(); recording; recording = Recordings->Next(recording))
        max_id = std::max(max_id, recording->Id());
      dsyslog("live, max rec id %d, number of recordings %d", max_id, Recordings->Count());
      if ((size_t)(max_id)+1 > RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].size())
        RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].resize(max_id+1, nullptr);
// for each vdr recording: compare with cached recording:
//   if equal: mark as seen
//   if not equal: delete
//   if not available (or just deleted); create, and mark as seen
      for (const cRecording* recording = Recordings->First(); recording; recording = Recordings->Next(recording)) {
        RecordingsItemRec *& live_rec = RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)][recording->Id()];
        if (live_rec && live_rec->has_changed(recording)) {
          dsyslog("live, recording %s has changed", recording->Name());
          delete live_rec;
          live_rec = nullptr;
        } // check is there is a recording, and if yes: has it changed?
        if (!live_rec) {
          change = true;
          live_rec = new RecordingsItemRec(recording);
          live_rec->finalize();
        }
      }  // for recording = Recordings->First(); ...
    }
// delete all non-seen cached recordings
    for (RecordingsItemRec*& r: all_recordings_iterator(iterator_begin() )) if (!r->seen()) {
      change = true;
      dsyslog("live cache: delete recording %s", r->NameStr().c_str() );
      delete r;
      r = nullptr;
    }
    if (!change) return;
// update sorted recs
    for (int sort_order = (int)RecordingsManager::eSortOrder::id + 1; sort_order < (int)RecordingsManager::eSortOrder::list_end; ++sort_order)
      RecordingsManager::all_recordings[sort_order].clear();
// add sorted by name
    RecordingsManager::all_recordings[(int)RecordingsManager::eSortOrder::name].assign(all_recordings_iterator(), all_recordings_iterator(iterator_end() ));

    std::sort(RecordingsManager::all_recordings[(int)RecordingsManager::eSortOrder::name].begin(), RecordingsManager::all_recordings[(int)RecordingsManager::eSortOrder::name].end(), &RecordingsItemPtrCompare::ByAscendingNameDescSort);

    time_update_all_recordings.stop();
    time_update_all_recordings.print("live: time_update_all_recordings ");
  }

  void update_all_recordings_scraper() {
    cMeasureTime time_update_all_recordings;
    {
      int all_rec_s = RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)].size();
      LOCK_RECORDINGS_READ;
      for (const cRecording* recording = Recordings->First(); recording; recording = Recordings->Next(recording)) {
        if (recording->Id() >= all_rec_s) continue;  // VDR has added recordings after last update of all_recordings
        RecordingsItemRec *live_rec = RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::id)][recording->Id()];
        if (!live_rec) continue;
        time_update_all_recordings.start();
        live_rec->updateScraperData(recording);
        time_update_all_recordings.stop();
      }
    }
    time_update_all_recordings.print("live: time_update_all_recordings_scraper ");
  }

  void RecordingsItemRec::getScraperData() {
    m_s_videoType = m_scraperVideo->getVideoType();   // sort duplicates
    m_s_dbid = m_scraperVideo->getDbId();             // sort duplicates
    if (m_s_videoType == tSeries || m_s_videoType == tMovie) {
      m_s_episode_number = m_scraperVideo->getEpisodeNumber(); // sort duplicates
      m_s_season_number = m_scraperVideo->getSeasonNumber();   // sort duplicates
      m_language = m_scraperVideo->getLanguage();              // sort duplicates
      if (m_timeOverview) m_timeOverview->start();
         // for TV show, we need m_s_title (name of folder)
         // for movie, we need m_s_collection_id
      m_scraperVideo->getOverview(&m_s_title, &m_s_episode_name, &m_s_release_date, &m_s_runtime, &m_s_IMDB_ID, &m_s_collection_id, nullptr);
      if (m_timeOverview) m_timeOverview->stop();
    }
  }
  void RecordingsItemRec::getScraperData(const cRecording *recording) {
    if (m_timeDurationDeviation) m_timeDurationDeviation->start();
    cGetScraperVideo_v01 getScraperVideo(nullptr, recording);
    if (m_timeIdentify) m_timeIdentify->start();
    bool scraper_available = getScraperVideo.call(LiveSetup().GetPluginTvscraper());
    if (m_timeIdentify) m_timeIdentify->stop();
    if (scraper_available) {
      m_scraperVideo.swap(getScraperVideo.m_scraperVideo);
      getScraperData();
    } else {
// service "GetScraperVideo_v01" is not available
      m_s_videoType = tNone;
    }
    if (m_timeDurationDeviation) m_timeDurationDeviation->stop();
  }
  void RecordingsItemRec::updateScraperData(const cRecording *recording) {
    if (!m_scraperVideo || !m_scraperVideo->has_changed(recording, m_s_runtime)) return;
// m_s_runtime is updated, and we do not cache duration deviation. There are no further changes -> return

// here we have to update our cached scraper data
    dsyslog("live, update scraper data for recording %s", recording->Name());
    getScraperData();
    if (m_s_videoType != tSeries && m_s_videoType != tMovie) {
      m_s_episode_number = 0;
      m_s_season_number = 0;
      m_language = 0;
      m_s_title.clear();
      m_s_episode_name.clear();
      m_s_release_date.clear();
      m_s_IMDB_ID.clear();
      m_s_collection_id = 0;
    }
    m_s_image = cTvMedia();
    m_s_image_requested = false;
    m_video_SD_HD = get_SD_HD(recording->Info() );
  }

  const cTvMedia &RecordingsItemRec::scraperImage() const {
    if (m_s_image_requested) return m_s_image; // return cached image
    m_s_image_requested = true;
// this is for the image in "overview". We allow as many levels as we have, to ensure that there is an image
//  m_imageLevels = cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::anySeasonCollection);
    if (m_scraperVideo) {
      m_s_image = m_scraperVideo->getImage(
        cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection),
        cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner), false);
    } else {
      if (!LiveSetup().GetTvscraperImageDir().empty() ) {
// there is a scraper plugin
        LOCK_RECORDINGS_READ;
        const cRecording* recording = Recordings->GetById(m_id);
        if (recording) EpgEvents::PosterTvscraper(m_s_image, nullptr, recording);
      }
    }
    return m_s_image;
  }

  bool RecordingsItemRec::matches_filter() const {
// check for global filter match
    if (!RecordingsManager::m_filter) return true;
    if (std::regex_search(Name().begin(), Name().end(), RecordingsManager::m_filter_regex)) return true;
    if (std::regex_search(ShortText().begin(), ShortText().end(), RecordingsManager::m_filter_regex)) return true;
    return std::regex_search(Description().begin(), Description().end(), RecordingsManager::m_filter_regex);
  }
  bool RecordingsItemRec::matches_regex(const std::regex *reg) const {
    if (!reg) return true;
    if (std::regex_search(Name().begin(), Name().end(), *reg)) return true;
    if (std::regex_search(ShortText().begin(), ShortText().end(), *reg)) return true;
    return std::regex_search(Description().begin(), Description().end(), *reg);
  }
  bool RecordingsItemRec::matchesFilter(cSv filter) const {
    if (filter.empty() ) return true;
    auto reg = getRegex(filter, g_locale, std::regex_constants::icase |
                                          std::regex_constants::nosubs |
                                          std::regex_constants::collate);
    return matches_regex(&reg);
  }

  int RecordingsItemRec::CompareTexts(const RecordingsItemRec *second, int *numEqualChars) const
// Compare Name + ShortText + Description
// if numEqualChars != NULL: return number of equal characters in ShortText + Description
  {
    if (numEqualChars) *numEqualChars = 0;
    int i = compare_utf8_lower_case_ignore_punct(m_name, second->m_name);
    if(i != 0) return i;
// name is identical, compare short text / description
    return CompareStD(second, numEqualChars);
  }

  int RecordingsItemRec::CompareStD(const RecordingsItemRec *second, int *numEqualChars) const
  {
// compare short text & description
    if (numEqualChars) *numEqualChars = 0;
    return compare_utf8_lower_case_ignore_punct(cUnion(m_shortText, m_description), cUnion(second->m_shortText, second->m_description), numEqualChars);
  }

  bool RecordingsItemRec::orderDuplicates(const RecordingsItemRec *second, bool alwaysShortText, bool lang) const {
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
        int i = compare_utf8_lower_case_ignore_punct(m_name, second->m_name);
        if (i != 0) return i < 0;
        if (!alwaysShortText) return false;
        return CompareStD(second) < 0;
    }
  }

  int RecordingsItemRec::get_SD_HD(const cRecordingInfo *info)
  {
// < -2: not checked. -1: Radio. 0 is SD, 1 is HD, >1 is UHD or better
    m_video_SD_HD = -2;
    if (m_scraperVideo) m_video_SD_HD = m_scraperVideo->getHD();
    if (m_video_SD_HD >= -1) return m_video_SD_HD;
    if (!info) {
      m_video_SD_HD = 0; // no information -> default to SD
      return m_video_SD_HD;
    }
// see ETSI EN 300 468, V1.15.1 (2016-03) or later, Chapter "6.2.8 Component Descriptor"
    const cComponents *components = info->Components();
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
    if (m_video_SD_HD < -1)  // nothing known found
    {
// also check frame rate for radio, as components are not reliable
      if (!videoStreamFound && audioStreamFound && info->FramesPerSecond() > 0 && info->FramesPerSecond() < 24)
        m_video_SD_HD = -1; // radio
      else
        m_video_SD_HD = 0; // no information -> SD as default
      if (info->ChannelName() ) {
        size_t l = strlen(info->ChannelName() );
        if( l > 3 && info->ChannelName()[l-2] == 'H' && info->ChannelName()[l-1] == 'D') m_video_SD_HD = 1;
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
//    if (IsArchived()) AppendHtmlEscapedAndCorrectNonUTF8(target, ArchiveDescr());
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
    switch (FrameWidth()) {
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
#endif
    if (!icon_name) icon_name = SD_HD() == 0 ? "sd": SD_HD() == 1 ? "hd": SD_HD() >= 2 ? "ud": "rd";
    target.append(icon_name);
    target.append("\", \"");
// [12] channel name
    AppendHtmlEscapedAndCorrectNonUTF8(target, ChannelName() );
    target.append("\", \"");
// [13] NewR()
    target.append(NewR() );
    target.append("\", \"");
// [14] Name
    AppendQuoteEscapedAndCorrectNonUTF8(target, Name() );
    target.append("\", \"");
// [15] Short text
    cSv text = ShortText();
    if (!text.empty() && Name() != text && !((Name().substr(0, 1) == "%" && Name().substr(1) == text)) ) AppendHtmlEscapedAndCorrectNonUTF8(target, text);
    target.append("\", \"");
// [16] Description
    AppendTextTruncateOnWord(target, Description(), LiveSetup().GetMaxTooltipChars(), true);
// [17] recording length deviation
    target.append("\",");
    target.concat(DurationDeviation());
// [18] Path / folder
    target.append(",\"");
    AppendHtmlEscapedAndCorrectNonUTF8(target, Folder() );
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
    StringAppendFrameParams(target, this);
    target.append("\"");
  }

  void RecordingsItemRec::AppendAsJSArray(cToSvConcat<0> &target, std::vector<RecordingsItemRec*>::const_iterator recIterFirst, std::vector<RecordingsItemRec*>::const_iterator recIterLast, bool &first, cSv filter, bool reverse) {
    if (reverse) {
      for (; recIterFirst != recIterLast;) {
        --recIterLast;
        RecordingsItemRec* recItem = *recIterLast;
        if (!recItem->matchesFilter(filter)) continue;
        if (!first) target.append(",");
        else first = false;
        target.concat(recItem->Id() );
      }
    } else {
      for (; recIterFirst != recIterLast; ++recIterFirst) {
        RecordingsItemRec* recItem = *recIterFirst;
        if (!recItem->matchesFilter(filter)) continue;
        if (!first) target.append(",");
        else first = false;
        target.concat(recItem->Id() );
      }
    }
  }

  bool operator< (const RecordingsItemDirPtr &a, const RecordingsItemDirPtr &b) { return *a < b; }
  bool operator< (cSv a, const RecordingsItemDirPtr &b) { return a < b->Name(); }
  bool operator< (const RecordingsItemDirPtr &a, cSv b) { return a->Name() < b; }
  bool operator< (int a, const RecordingsItemDirPtr &b) { return *b > a; }
  bool operator< (const RecordingsItemDirPtr &a, int b) { return *a < b; }

template<class C> std::vector<C*> &get_default_items() {
  return std::vector<C*>();
}
template <> std::vector<RecordingsItemRec*> &get_default_items() {
  return RecordingsManager::all_recordings_sorted;
}
/*
template <> std::vector<c_filesystem_dir*> &get_default_items() {
  return RecordingsManager::all_directories;
}
*/


template<class C>
  const_rec_iterator<C>::const_rec_iterator(std::vector<C*> &items, bool backwards, std::regex *regex_filter):
    m_backwards(backwards), m_regex_filter(regex_filter)
  {
    set_container(items);
  }
template<class C>
  const_rec_iterator<C>::const_rec_iterator():
    m_backwards(RecordingsManager::m_backwards),
    m_regex_filter(RecordingsManager::m_filter?(&RecordingsManager::m_filter_regex):(std::regex *)nullptr)
  {
    set_container(get_default_items<C>() );
  }
template<class C>
  const_rec_iterator<C> &const_rec_iterator<C>::set_recordingsItemDir(RecordingsItemDirPtr &recordingsItemDir) {
    m_recordingsItemDir = recordingsItemDir;
    m_recordingsItemDir->set_rec_iterator(*this);
    return *this;
  }

template<class C>
  const_rec_iterator<C> &const_rec_iterator<C>::set_max_items(unsigned max_items) {
    m_max_items = max_items;
    return *this;
  }
template<class C>
  const_rec_iterator<C> &const_rec_iterator<C>::set_container(std::vector<C*> &items) {
    m_begin = items.begin();
    m_end   = items.end();
    return set_begin();
  }
template<class C>
  const_rec_iterator<C> &const_rec_iterator<C>::set_begin() {
    m_index = 0;
    if (m_backwards) {
      m_current = m_end;
      m_backwards_end = false;
      ++(*this);
    } else {
      m_current = m_begin;
      if (!current_matches() ) ++(*this);
      else m_index = 1;
    }
    return *this;
  }
template<class C>
  unsigned const_rec_iterator<C>::count() {
    while (*this != iterator_end() ) ++(*this);
    unsigned count = m_index;
    set_begin();
    return count;
  }
template<class C>
  const_rec_iterator<C> &const_rec_iterator<C>::operator++() {
    if (m_backwards) {
      while(m_current != m_begin) {
        --m_current;
        if (current_matches() ) { ++m_index; return *this; }
      }
      m_backwards_end = true;
    } else {
      for(++m_current; m_current != m_end; ++m_current) {
        if (current_matches() ) { ++m_index; return *this; }
      }
    }
    return *this;
  }
template<class C>
  bool const_rec_iterator<C>::operator==(iterator_end d) const {
    return ( m_backwards & m_backwards_end) |
           (!m_backwards & (m_current == m_end)) |
           ( m_index >= m_max_items);
  }
template<class C>
  bool const_rec_iterator<C>::current_matches() const {
    if (!*m_current) return false;
    if (m_recordingsItemDir)
      if (!m_recordingsItemDir->Contains(*m_current)) return false;

    return (*m_current)->matches_regex(m_regex_filter);
  }
// template class const_rec_iterator<c_basic_dir>;
// template class const_rec_iterator<c_filesystem_dir>;
template class const_rec_iterator<RecordingsItemRec>;


  bool c_basic_dir::matches_regex(const std::regex *reg) const {
    if (!reg) return true;
    if (std::regex_search(Name().begin(), Name().end(), *reg)) return true;
    for (auto r:const_rec_iterator<RecordingsItemRec>(RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::name)], false))
      if(r->matches_regex(reg)) return true;
// TODO implement for non-filesystem dirs
    return false;
  }
  bool c_basic_dir::ContainsNew() const {
    for (auto r:const_rec_iterator<RecordingsItemRec>(RecordingsManager::all_recordings[(int)(RecordingsManager::eSortOrder::name)], false))
      if(r->IsNew()) return true;
// TODO implement for non-filesystem dirs
    return false;
  }

// c_filesystem_dir
  void c_filesystem_dir::add_if_not_exists(cSv name_vdr) {
/*
    if (name_vdr.empty()) return; // do not add root dir here
    for (c_basic_dir *basic_dir: RecordingsManager::all_directories)
      if (basic_dir->Type() == RecordingsManager::eDirType::filesystem &&
          static_cast<c_filesystem_dir*>(basic_dir)->Name_vdr() == name_vdr) return;
// not yet in list, so add it now
    c_filesystem_dir *filesystem_dir = new c_filesystem_dir(name_vdr);
    RecordingsManager::all_directories.push_back(filesystem_dir);
    add_if_not_exists(filesystem_dir->Folder());
*/
  }
  c_filesystem_dir::c_filesystem_dir(cSv name_vdr):
    c_basic_dir(RecordingsManager::eDirType::filesystem),
    m_name_vdr(std::string(name_vdr)) {
// get m_name, m_name_str, m_folder
    const_split_iterator name_split(iterator_end(), m_name_vdr, '~');
    --name_split;
    m_name = *name_split;
    m_name_str = name_split.pos();
    if (name_split != iterator_begin()) {
      m_folder = cSv(m_name_vdr.data(), std::distance((const char *)m_name_vdr.data(), name_split.pos())-1);
    }
    m_level = name_split.size() - 1;
  }
  bool c_filesystem_dir::Contains(RecordingsItemRec* recordingsItemRec) const {
    return m_name_vdr == recordingsItemRec->Folder();
  }
  bool c_filesystem_dir::Contains(c_basic_dir* basic_dir) const {
    return Type() == basic_dir->Type() &&
           m_name_vdr == static_cast<c_filesystem_dir*>(basic_dir)->Folder();
  }

  /**
   *  Implementation of class RecordingsTree:
   */
  RecordingsTree::RecordingsTree():
    m_maxLevel(0),
    m_root(std::make_shared<RecordingsItemDir>("", 0))
  {
//   esyslog("live: DH: ****** RecordingsTree::RecordingsTree() ********");
    cMeasureTime timeRecs, timeIdentify, timeOverview, timeDurationDeviation, timeItemRec;

// check availability of scraper data
    m_creation_timestamp = time(0);
    cGetScraperVideo_v01 getScraperVideo;
    bool scraperDataAvailable = getScraperVideo.call(LiveSetup().GetPluginTvscraper());
    RecordingsItemDirPtr recPtrTvShows = std::make_shared<RecordingsItemDir>(tr("TV shows"), 1);
    RecordingsItemDirPtr recPtrMovieCollections = std::make_shared<RecordingsItemDir>(tr("Movie collections"), 1);
// create "base" folders
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
    for (RecordingsItemRec *recPtr: all_recordings_iterator() ) {
      if (scraperDataAvailable) m_maxLevel = std::max(m_maxLevel, recPtr->HierarchyLevels() + 1);
      else m_maxLevel = std::max(m_maxLevel, recPtr->HierarchyLevels() );

      RecordingsItemDirPtr dir = m_rootFileSystem;
      if (!recPtr->Folder().empty())
        for (cSv folderPart: cSplit(recPtr->Folder(), FOLDERDELIMCHAR)) {
          dir = dir->addDirIfNotExists(folderPart);
        }

      dir->m_entries.push_back(recPtr);
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
    timeDurationDeviation.print("live: Scraper   ");
*/
  }

  RecordingsTree::~RecordingsTree()
  {
    // esyslog("live: DH: ****** RecordingsTree::~RecordingsTree() ********");
  }
  const std::vector<RecordingsItemRec*> *RecordingsTree::allRecordings(RecordingsManager::eSortOrder sortOrder) {
    std::vector<RecordingsItemRec*> &l_all_recordings_sorted = RecordingsManager::all_recordings[(int)(sortOrder)];
    if (l_all_recordings_sorted.empty() ) {
      l_all_recordings_sorted.assign(all_recordings_iterator(), all_recordings_iterator(iterator_end() ));
      std::sort(l_all_recordings_sorted.begin(), l_all_recordings_sorted.end(), RecordingsItemPtrCompare::getComp(sortOrder));
    }
    return &l_all_recordings_sorted;
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
bool ByScraperDataAvailable(const RecordingsItemRec *first, tvType videoType) {
  return (int)first->scraperVideoType() < (int)videoType; // tNone if no scraper data. Move these to the end, by using <
}

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where NO scraper data are available.
  const std::vector<RecordingsItemRec*> *recItems = RecordingsTree->allRecordings(RecordingsManager::eSortOrder::duplicatesLanguage);
// sorting for duplicatesLanguage is OK, even if language is not required here (language is ignored later)
  std::vector<RecordingsItemRec*>::const_iterator currentRecItem, recIterUpName, recIterUpShortText, recIterLowShortText;
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

void addDuplicateRecordingsLang(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree) {
// add duplicate recordings where scraper data are available.
// add only recordings with different language
  const std::vector<RecordingsItemRec*> *recItems = RecordingsTree->allRecordings(RecordingsManager::eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemRec*>::const_iterator currentRecItem, nextRecItem, recIterUpName, recIterLowName;

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
void addDuplicateRecordingsSd(std::vector<RecordingsItemRec*> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where scraper data are available.
// recordings with different languages are NOT duplicates
  const std::vector<RecordingsItemRec*> *recItems = RecordingsTree->allRecordings(RecordingsManager::eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemRec*>::const_iterator currentRecItem, recIterUpName, recIterLowName;

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
