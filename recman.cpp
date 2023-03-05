#ifdef HAVE_PCRE2
#include "StringMatch.h"
#endif
#include <iostream>

#include "recman.h"
#include "tools.h"
#include "services.h"
#include "epg_events.h"


// STL headers need to be before VDR tools.h (included by <vdr/videodir.h>)
#include <fstream>
#include <stack>
#include <algorithm>

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
#if VDRVERSNUM >= 20301
	cStateKey RecordingsManager::m_recordingsStateKey;
#else
	int RecordingsManager::m_recordingsState = 0;
#endif
        time_t scraperLastRecordingsUpdate;

	// The RecordingsManager holds a VDR lock on the
	// Recordings. Additionally the singleton instance of
	// RecordingsManager is held in a weak pointer. If it is not in
	// use any longer, it will be freed automaticaly, which leads to a
	// release of the VDR recordings lock. Upon requesting access to
	// the RecordingsManager via LiveRecordingsManger function, first
	// the weak ptr is locked (obtaining a std::shared_ptr from an possible
	// existing instance) and if not successfull a new instance is
	// created, which again locks the VDR Recordings.
	//
	// RecordingsManager provides factory methods to obtain other
	// recordings data structures. The data structures returned keep if
	// needed the instance of RecordingsManager alive until destructed
	// themselfs. This way the use of LIVE::recordings is straight
	// forward and does hide the locking needs from the user.

#if VDRVERSNUM >= 20301
	RecordingsManager::RecordingsManager()
#else
	RecordingsManager::RecordingsManager() :
		m_recordingsLock(&Recordings)
#endif
	{
	}

	RecordingsTreePtr RecordingsManager::GetRecordingsTree() const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsTreePtr(recMan, std::shared_ptr<RecordingsTree>());
		}
		return RecordingsTreePtr(recMan, m_recTree);
	}

	std::string RecordingsManager::Md5Hash(cRecording const * recording) const
	{
		return "recording_" + MD5Hash(recording->FileName());
	}

	cRecording const * RecordingsManager::GetByMd5Hash(std::string const & hash) const
	{
		if (!hash.empty()) {
#if VDRVERSNUM >= 20301
			LOCK_RECORDINGS_READ;
			for (cRecording* rec = (cRecording *)Recordings->First(); rec; rec = (cRecording *)Recordings->Next(rec)) {
#else
			for (cRecording* rec = Recordings.First(); rec; rec = Recordings.Next(rec)) {
#endif
				if (hash == Md5Hash(rec))
					return rec;
			}
		}
		return 0;
	}

	bool RecordingsManager::MoveRecording(cRecording const * recording, std::string const & name, bool copy) const
	{
		if (!recording)
			return false;

		std::string oldname = recording->FileName();
		size_t found = oldname.find_last_of("/");

		if (found == std::string::npos)
			return false;

#if APIVERSNUM > 20101
		std::string newname = std::string(cVideoDirectory::Name()) + "/" + name + oldname.substr(found);
#else
		std::string newname = std::string(VideoDirectory) + "/" + name + oldname.substr(found);
#endif

		if (!MoveDirectory(oldname.c_str(), newname.c_str(), copy)) {
			esyslog("live: renaming failed from '%s' to '%s'", oldname.c_str(), newname.c_str());
			return false;
		}

#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_WRITE;
		if (!copy)
			Recordings->DelByName(oldname.c_str());
		Recordings->AddByName(newname.c_str());
#else
		if (!copy)
			Recordings.DelByName(oldname.c_str());
		Recordings.AddByName(newname.c_str());
#endif
		cRecordingUserCommand::InvokeCommand(*cString::sprintf("rename \"%s\"", *strescape(oldname.c_str(), "\\\"$'")), newname.c_str());

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
#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_WRITE;
		Recordings->DelByName(name.c_str());
#else
		Recordings.DelByName(name.c_str());
#endif
	}

	int RecordingsManager::GetArchiveType(cRecording const * recording)
	{
		std::string filename = recording->FileName();

		std::string dvdFile = filename + "/dvd.vdr";
		if (0 == access(dvdFile.c_str(), R_OK)) {
			return 1;
		}
		std::string hddFile = filename + "/hdd.vdr";
		if (0 == access(hddFile.c_str(), R_OK)) {
			return 2;
		}
		return 0;
	}

	std::string const RecordingsManager::GetArchiveId(cRecording const * recording, int archiveType)
	{
		std::string filename = recording->FileName();

		if (archiveType==1) {
			std::string dvdFile = filename + "/dvd.vdr";
			std::ifstream dvd(dvdFile.c_str());

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
			std::ifstream hdd(hddFile.c_str());

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

#if VDRVERSNUM >= 20301
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
#endif

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

		// StateChanged must be executed every time, so not part of
		// the short cut evaluation in the if statement below.
#if VDRVERSNUM >= 20301
		bool stateChanged = StateChanged();
#else
		bool stateChanged = Recordings.StateChanged(m_recordingsState);
#endif
// check: changes on scraper data?
                cGetScraperUpdateTimes scraperUpdateTimes;
                if (scraperUpdateTimes.call(LiveSetup().GetPluginScraper()) ) {
                  if (scraperUpdateTimes.m_recordingsUpdateTime > scraperLastRecordingsUpdate) {
                    scraperLastRecordingsUpdate = scraperUpdateTimes.m_recordingsUpdateTime;
                    stateChanged = true;
                  }
                }
		if (stateChanged || (!m_recTree) ) {
			if (stateChanged) {
				m_recTree.reset();
			}
			if (stateChanged || !m_recTree) {
				m_recTree = std::shared_ptr<RecordingsTree>(new RecordingsTree(recMan));
			}
			if (!m_recTree) {
				esyslog("live: creation of recordings tree failed!");
				return RecordingsManagerPtr();
			}

		}
		return recMan;
	}


        ShortTextDescription::ShortTextDescription(const char * ShortText, const char * Description) :
            m_short_text(ShortText),
            m_description(Description)
           {  }
        wint_t ShortTextDescription::getNextNonPunctChar() {
           wint_t result;
           do {
             if( m_short_text && *m_short_text ) result = getNextUtfCodepoint(m_short_text);
				            else result = getNextUtfCodepoint(m_description);
           } while (result && iswpunct(result));
           return towlower(result);
        }

	/**
	 * Implemetation of class RecordingsItemPtrCompare
	 */

  int RecordingsItemPtrCompare::FindBestMatch(RecordingsItemPtr & BestMatch, const std::vector<RecordingsItemPtr>::const_iterator & First, const std::vector<RecordingsItemPtr>::const_iterator & Last, const RecordingsItemPtr & EPG_Entry){
// d: length of movie in minutes, without commercial breaks
// Assumption: the maximum length of commercial breaks is cb% * d
// Assumption: cb = 34%
   const long cb = 34;
// lengthLowerBound: minimum of d, if EPG_Entry->Duration() has commercial breaks
   long lengthLowerBound = EPG_Entry->Duration() * 100 / ( 100 + cb) ;
// lengthUpperBound: if EPG_Entry->Duration() is d (no commercial breaks), max length of recording with commercial breaks
// Add VDR recording margins to this value
   long lengthUpperBound = EPG_Entry->Duration() * (100 + cb) / 100;
   lengthUpperBound += ::Setup.MarginStart + ::Setup.MarginStop;
   if(EPG_Entry->Duration() >= 90 && lengthLowerBound < 70) lengthLowerBound = 70;

   int numRecordings = 0;
   int min_deviation = 100000;
   std::vector<RecordingsItemPtr>::const_iterator bestMatchIter;
   for ( std::vector<RecordingsItemPtr>::const_iterator iter = First; iter != Last; ++iter)
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

	bool RecordingsItemPtrCompare::ByAscendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second)
	{
		return (first->StartTime() < second->StartTime());
	}

	bool RecordingsItemPtrCompare::ByDuplicatesName(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           return first->orderDuplicates(second, false);
	}

	bool RecordingsItemPtrCompare::ByDuplicates(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
          return first->orderDuplicates(second, true);
	}

	bool RecordingsItemPtrCompare::ByDuplicatesLanguage(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
          return first->orderDuplicates(second, true, true);
	}

int firstNonPunct(const std::string &s) {
// returns first non-punct char in s
  unsigned int ret;
  for (ret = 0; ret < s.length() && ispunct(s[ret]); ret++ );
  return ret;
}
	bool RecordingsItemPtrCompare::ByAscendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
// used for sort
	{
           int start_f = firstNonPunct(first->Name() );
           int start_s = firstNonPunct(second->Name() );
// see https://en.cppreference.com/w/cpp/locale/collate/compare
// Compares the character sequence [low1, high1) to the character sequence [low2, high2)
           int i = g_collate_char.compare(&first->Name()[start_f], &first->Name()[0] + first->Name().size(),
                                         &second->Name()[start_s], &second->Name()[0] + second->Name().size() );
           if(i != 0) return i < 0;
           return first->CompareStD(second) < 0;
	}

        bool RecordingsItemPtrCompare::ByDescendingRecordingErrors(const RecordingsItemPtr & first, const RecordingsItemPtr & second){
/*
           print("ByDescendingRecordingErrors, first: ", first);
           print(" sec: ", second);
           std::cout << "\n";
*/
           return first->RecordingErrors() > second->RecordingErrors();
        }
        bool RecordingsItemPtrCompare::ByDescendingDurationDeviation(const RecordingsItemPtr & first, const RecordingsItemPtr & second) {
          return first->DurationDeviation() > second->DurationDeviation();
	      }

	bool RecordingsItemPtrCompare::ByEpisode(const RecordingsItemPtr & first, const RecordingsItemPtr & second) {
	  return first->scraperEpisodeNumber() < second->scraperEpisodeNumber();
	}

	bool RecordingsItemPtrCompare::BySeason(const RecordingsItemPtr & first, const RecordingsItemPtr & second) {
	  return first->scraperSeasonNumber() < second->scraperSeasonNumber();
	}

	bool RecordingsItemPtrCompare::ByReleaseDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second) {
	  return first->scraperReleaseDate() < second->scraperReleaseDate();
	}

        int RecordingsItemPtrCompare::compareLC(const char *first, const char *second, int *numEqualChars) {
// if numEqualChars != NULL: Add the number of equal characters to *numEqualChars
          bool fe = !first  || !first[0];    // is first  string empty string?
          bool se = !second || !second[0];   // is second string empty string?
          if (fe && se) return 0;
          if (se) return  1;
          if (fe) return -1;
// compare strings case-insensitive
          for(; *first && *second; ) {
            wint_t  flc = towlower(getNextUtfCodepoint(first));
            wint_t  slc = towlower(getNextUtfCodepoint(second));
            if ( flc < slc ) return -1;
            if ( flc > slc ) return  1;
            if (numEqualChars) (*numEqualChars)++;
          }
          if (*second ) return -1;
          if (*first  ) return  1;
          return 0;
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


bool searchNameDesc(RecordingsItemPtr &RecItem, const std::vector<RecordingsItemPtr> *RecItems, const cEvent *event, cScraperVideo *scraperVideo) {
  if (RecItems->empty() ) return false;  // there are no recordings

// find all recordings with equal name
  RecordingsItemPtr dummy (new RecordingsItemDummy(event, scraperVideo));
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
  if (num_match_rec == 0 || num_match_rec > 5) return false; // no matching lenght or series (too many matching length)
  return true;
}

	/**
	 *  Implementation of class RecordingsItem:
	 */
	RecordingsItem::RecordingsItem(std::string const & name) :
		m_name(name),
		m_name_for_search(RecordingsItem::GetNameForSearch(name))
	{
	}

	RecordingsItem::~RecordingsItem()
	{
	}

  std::string RecordingsItem::GetNameForSearch(std::string const & name)
	{
    std::string result;
    result.reserve(name.length());
    const char *name_c = name.c_str();
    while(*name_c) {
      wint_t codepoint = getNextUtfCodepoint(name_c);
      if(!iswpunct(codepoint) ) AppendUtfCodepoint(result, towlower(codepoint));
    }
    return result;
	}

	int RecordingsItem::numberOfRecordings() {
    int result = m_entries.size();
	  for (const auto &item: m_subdirs) result += item->numberOfRecordings();
	  return result;
  }

	void RecordingsItem::finishRecordingsTree() {
	  for (auto &item: m_subdirs) item->finishRecordingsTree();
	  if (m_cmp_rec) std::sort(m_entries.begin(), m_entries.end(), m_cmp_rec);
	  if (m_cmp_dir) std::sort(m_subdirs.begin(), m_subdirs.end(), m_cmp_dir);
	  else std::sort(m_subdirs.begin(), m_subdirs.end(), RecordingsItemPtrCompare::ByAscendingNameDescSort);
	  m_entries.shrink_to_fit();
	  m_subdirs.shrink_to_fit();
  }

  RecordingsItemPtr RecordingsItem::addDirIfNotExists(const std::string &dirName) {
    std::vector<RecordingsItemPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), dirName);
    if (iter != m_subdirs.end() && !(dirName < *iter) ) return *iter;
	  RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, Level() + 1));
	  m_subdirs.insert(iter, recPtr);
    return recPtr;
	}

  RecordingsItemPtr RecordingsItem::addDirCollectionIfNotExists(int collectionId, const cRecording* recording) {
    std::vector<RecordingsItemPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), collectionId);
    if (iter != m_subdirs.end() && !(collectionId < *iter) ) return *iter;
	  RecordingsItemPtr recPtr2 (new RecordingsItemDirCollection(Level() + 1, recording));
	  m_subdirs.insert(iter, recPtr2);
    return recPtr2;
	}

  RecordingsItemPtr RecordingsItem::addDirSeasonIfNotExists(int seasonNumber, const cRecording* recording) {
    std::vector<RecordingsItemPtr>::iterator iter = std::lower_bound(m_subdirs.begin(), m_subdirs.end(), seasonNumber);
    if (iter != m_subdirs.end() && !(seasonNumber < *iter) ) return *iter;
	  RecordingsItemPtr recPtr2 (new RecordingsItemDirSeason(Level() + 1, recording));
	  m_subdirs.insert(iter, recPtr2);
    return recPtr2;
	}

	const std::vector<RecordingsItemPtr> *RecordingsItem::getRecordings(eSortOrder sortOrder)
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

  bool RecordingsItem::matchesFilter(const std::string &filter) {
    if (filter.empty() ) return true;
#ifdef HAVE_PCRE2
    StringMatch sm(filter);
    return sm.Matches(Name()) or
      sm.Matches(ShortText()?ShortText() : "" ) or
      sm.Matches(Description()?Description() : "");
#endif
    return true;
  }

  bool RecordingsItem::checkNew() {
	  if (Recording() && Recording()->GetResume() <= 0) return true;
	  for (const auto &rec:m_entries)
	    if (rec->Recording() && rec->Recording()->GetResume() <= 0) return true;
	  for (const auto &subdir:m_subdirs) if (subdir->checkNew() ) return true;
	  return false;
	}
  void RecordingsItem::addDirList(std::vector<std::string> &dirs, const std::string &basePath)
	{
	  if (!IsDir() ) return;
          std::string basePath0 = basePath;
          if (basePath.empty() ) dirs.push_back("");
          else basePath0.append("/");
	  for (const auto &subdir:m_subdirs) {
            std::string thisPath = basePath0 + subdir->m_name;
            dirs.push_back(thisPath);
            subdir->addDirList(dirs, thisPath);
          }
	}

	void RecordingsItem::setTvShow(const cRecording* recording) {
          if (m_cmp_dir) return;
          m_cmp_dir = RecordingsItemPtrCompare::BySeason;
          getScraperData(recording, cImageLevels(eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection) );
	}

	void RecordingsItem::getScraperData(const cRecording* recording, const cImageLevels &imageLevels, std::string *collectionName) {
    cGetScraperVideo getScraperVideo(NULL, recording);
    if (getScraperVideo.call(LiveSetup().GetPluginScraper()) ) {
      m_s_videoType = getScraperVideo.m_scraperVideo->getVideoType();
      m_s_dbid = getScraperVideo.m_scraperVideo->getDbId();
      getScraperVideo.m_scraperVideo->getOverview(&m_s_title, &m_s_episode_name, &m_s_release_date, &m_s_runtime, &m_s_IMDB_ID, &m_s_collection_id, collectionName);
      m_s_episode_number = getScraperVideo.m_scraperVideo->getEpisodeNumber();
      m_s_season_number = getScraperVideo.m_scraperVideo->getSeasonNumber();
      m_s_image = getScraperVideo.m_scraperVideo->getImage(imageLevels, cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner), false);
      m_duration_deviation = getScraperVideo.m_scraperVideo->getDurationDeviation();
      m_language = getScraperVideo.m_scraperVideo->getLanguage();
      m_video_SD_HD = getScraperVideo.m_scraperVideo->getHD();
    } else {
// service "GetScraperVideo" is not available, just get the image
      m_s_videoType = tNone;
      EpgEvents::PosterTvscraper(m_s_image, NULL, recording);
    }
	}

	int RecordingsItem::CompareTexts(const RecordingsItemPtr &second, int *numEqualChars) const
// Compare NameForSearch + ShortText + Description
// if numEqualChars != NULL: return namber of equal characters in ShortText + Description
	{
          if (numEqualChars) *numEqualChars = 0;
          int i = NameForSearch().compare(second->NameForSearch() );
          if(i != 0) return i;
// name is identical, compare short text
          i = RecordingsItemPtrCompare::compareLC(ShortText(), second->ShortText(), numEqualChars);
          if(i != 0) return i;
          i = RecordingsItemPtrCompare::compareLC(Description(), second->Description(), numEqualChars);
          return i;
	}

        int RecordingsItem::CompareStD(const RecordingsItemPtr &second, int *numEqualChars) const
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

        bool RecordingsItem::orderDuplicates(const RecordingsItemPtr &second, bool alwaysShortText, bool lang) const {
// this is a < operation. Return false in case of ==   !!!!!
// lang is the last criterium. For sorting, you can always set lang == true
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
              return RecordingsItemPtrCompare::compareLC(ShortText(), second->ShortText() ) < 0;
          }
        }

	/**
	 *  Implementation of class RecordingsItemDir:
	 */
	RecordingsItemDir::RecordingsItemDir(const std::string& name, int level):
		RecordingsItem(name),
                m_level(level)
	{
		// dsyslog("live: REC: C: dir %s -> %s", name.c_str(), parent ? parent->Name().c_str() : "ROOT");
	}

	RecordingsItemDir::~RecordingsItemDir()
	{
		// dsyslog("live: REC: D: dir %s", Name().c_str());
	}

	/**
	 *  Implementation of class RecordingsItemDirCollection:
	 */
	RecordingsItemDirCollection::RecordingsItemDirCollection(int level, const cRecording* recording):
		RecordingsItemDir("", level)
	{
          m_cmp_rec = RecordingsItemPtrCompare::ByReleaseDate;
          getScraperData(recording, cImageLevels(eImageLevel::tvShowCollection, eImageLevel::seasonMovie), &m_name);
	}

	RecordingsItemDirCollection::~RecordingsItemDirCollection() { }

	/**
	 *  Implementation of class RecordingsItemDirSeason:
	 */
	RecordingsItemDirSeason::RecordingsItemDirSeason(int level, const cRecording* recording):
		RecordingsItemDir("", level)
	{
    m_cmp_rec = RecordingsItemPtrCompare::ByEpisode;
    getScraperData(recording, cImageLevels(eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection) );
	  m_name = std::to_string(m_s_season_number);
	}
	RecordingsItemDirSeason::~RecordingsItemDirSeason() { }

	/**
	 *  Implementation of class RecordingsItemRec:
	 */
	RecordingsItemRec::RecordingsItemRec(int idI, const std::string& id, const std::string& name, const cRecording* recording):
		RecordingsItem(name),
		m_recording(recording),
		m_id(id),
		m_isArchived(RecordingsManager::GetArchiveType(m_recording) )
	{
    // dsyslog("live: REC: C: rec %s -> %s", name.c_str(), parent->Name().c_str());
    m_idI = idI;
    getScraperData(recording,
      cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection));
	}

	RecordingsItemRec::~RecordingsItemRec()
	{
		// dsyslog("live: REC: D: rec %s", Name().c_str());
	}

template<class T>
        void RecordingsItem::AppendShortTextOrDesc(T &target) const
// note: only up to next line break, with limited length and html escaped, and UTF8 corrected
        {
          if (! RecInfo() ) return;
          const char *text = ShortText();
          if (!text || Name() == text ) text = RecInfo()->Description();
          AppendTextMaxLen(target, text);
        }
template void RecordingsItem::AppendShortTextOrDesc<std::string>(std::string &target) const;
template void RecordingsItem::AppendShortTextOrDesc<cLargeString>(cLargeString &target) const;

        int RecordingsItemRec::SD_HD()
        {
           if ( m_video_SD_HD >= 0 ) return m_video_SD_HD;
           const cComponents *components = RecInfo()->Components();
           if(components) for( int ix = 0; ix < components->NumComponents(); ix++) {
             tComponent * component = components->Component(ix);
             if (component->stream == 1 || component->stream == 5) {
               switch (component->type) {
                 case 1:
                 case 5: m_video_SD_HD = 0; break;
                 case 2:
                 case 3:
                 case 6:
                 case 7: m_video_SD_HD = 0; break;
                 case 4:
                 case 8: m_video_SD_HD = 0; break;
                 case 9:
                 case 13: m_video_SD_HD = 1; break;
                 case 10:
                 case 11:
                 case 14:
                 case 15: m_video_SD_HD = 1; break;
                 case 12:
                 case 16: m_video_SD_HD = 1; break;
               }
             }
           }
           if(m_video_SD_HD == -1)
             {
             m_video_SD_HD = 0;
             if(RecInfo()->ChannelName() ) {
               size_t l = strlen(RecInfo()->ChannelName() );
               if( l > 3 && RecInfo()->ChannelName()[l-2] == 'H' && RecInfo()->ChannelName()[l-1] == 'D') m_video_SD_HD = 1;
               }
             }
          return m_video_SD_HD;
        }

void AppendScraperData(cLargeString &target, const std::string &s_IMDB_ID, const cTvMedia &s_image, tvType s_videoType, const std::string &s_title, int s_season_number, int s_episode_number, const std::string &s_episode_name, int s_runtime, const std::string &s_release_date) {
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
          if (scraperDataAvailable) AppendHtmlEscapedAndCorrectNonUTF8(target, s_title.c_str() );
          target.append("\",\"");
          if (s_videoType == tSeries && (s_episode_number != 0 || s_season_number != 0)) {
// [6] : season/episode/episode name (scraper)
            target.append(s_season_number);
            target.append('E');
            target.append(s_episode_number);
            target.append(' ');
            AppendHtmlEscapedAndCorrectNonUTF8(target, s_episode_name.c_str() );
          }
          target.append("\",\"");
// [7] : runtime (scraper)
          if (scraperDataAvailable && s_runtime) AppendDuration(target, tr("(%d:%02d)"), s_runtime / 60, s_runtime % 60);
          target.append("\",\"");
// [8] : relase date (scraper)
          if (scraperDataAvailable && !s_release_date.empty() ) target.append(s_release_date);
          target.append("\"");
	}

        void RecordingsItemRec::AppendAsJSArray(cLargeString &target, bool displayFolder){
          target.append("\"");
// [0] : ID
          target.append(Id().c_str() + 10);
          target.append("\",\"");
// [1] : ArchiveDescr()
          if (IsArchived()) AppendHtmlEscapedAndCorrectNonUTF8(target, ArchiveDescr().c_str() );
          target.append("\",");
// scraper data
          AppendScraperData(target, m_s_IMDB_ID, m_s_image, m_s_videoType, m_s_title, m_s_season_number, m_s_episode_number, m_s_episode_name, m_s_runtime, m_s_release_date);
// [9] : Day, time & duration
          target.append(",\"");
          AppendDateTime(target, tr("%a,"), StartTime());  // day of week
          target.append(' ');
          AppendDateTime(target, tr("%b %d %y"), StartTime());  // date
          target.append(' ');
          AppendDateTime(target, tr("%I:%M %p"), StartTime() );  // time
          if(Duration() >= 0) {
            target.append(' ');
            AppendDuration(target, tr("(%d:%02d)"), Duration() / 60, Duration() % 60);
          }
          target.append("\", ");
// RecordingErrors, Icon
#if VDRVERSNUM >= 20505
// [10] : Number of recording errors
          target.append(RecordingErrors() );
#else
          target.append("-100");
#endif
          target.append(", \"");
// [11] HD_SD
          target.append(SD_HD() == 0 ? 's': SD_HD() == 1 ? 'h': 'u');
          target.append("\", \"");
// [12] channel name
          AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->ChannelName() );
          target.append("\", \"");
// [13] NewR()
          target.append(NewR() );
          target.append("\", \"");
// [14] Name
          AppendHtmlEscapedAndCorrectNonUTF8(target, Name().c_str() );
          target.append("\", \"");
// [15] Short text
          const char *text = ShortText();
          if (text && Name() != text ) AppendHtmlEscapedAndCorrectNonUTF8(target, text);
          target.append("\", \"");
// [16] Description
          AppendTextTruncateOnWord(target, RecInfo()->Description(), LiveSetup().GetMaxTooltipChars(), true);
// [17] recording length deviation
          target.append("\",");
          target.append(DurationDeviation());
// [18] Path / folder
          if(displayFolder) {
            target.append(",\"");
            if( *(const char *)Recording()->Folder() ) AppendHtmlEscapedAndCorrectNonUTF8(target, (const char *)Recording()->Folder() );
            target.append("\"");
          }
        }

void RecordingsItemRec::AppendAsJSArray(cLargeString &target, std::vector<RecordingsItemPtr>::const_iterator recIterFirst, std::vector<RecordingsItemPtr>::const_iterator recIterLast, bool &first, const std::string &filter, bool reverse) {
  if (reverse) {
    for (; recIterFirst != recIterLast;) {
      --recIterLast;
      RecordingsItemPtr recItem = *recIterLast;
      if (!recItem->matchesFilter(filter)) continue;
      if (!first) target.append(",");
      else first = false;
      target.append(recItem->IdI() );
    }
  } else {
    for (; recIterFirst != recIterLast; ++recIterFirst) {
      RecordingsItemPtr recItem = *recIterFirst;
      if (!recItem->matchesFilter(filter)) continue;
      if (!first) target.append(",");
      else first = false;
      target.append(recItem->IdI() );
    }
  }
}

	/**
	 * Implemetation of class RecordingsItemDummy
	 */
  RecordingsItemDummy::RecordingsItemDummy(const cEvent *event, cScraperVideo *scraperVideo):
    RecordingsItem(charToString(event->Title() )),
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

  bool operator< (const RecordingsItemPtr &a, const RecordingsItemPtr &b) { return *a < b; }
  bool operator< (const std::string &a, const RecordingsItemPtr &b) { return a < b->Name(); }
  bool operator< (const RecordingsItemPtr &a, const std::string &b) { return a->Name() < b; }
  bool operator< (int a, const RecordingsItemPtr &b) { return *b > a; }
  bool operator< (const RecordingsItemPtr &a, int b) { return *a < b; }

	/**
	 *  Implementation of class RecordingsTree:
	 */
	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir("", 0))
	{
// esyslog("live: DH: ****** RecordingsTree::RecordingsTree() ********");
// check availability of scraper data
    cGetScraperVideo getScraperVideo;
		bool scraperDataAvailable = getScraperVideo.call(LiveSetup().GetPluginScraper());
    RecordingsItemPtr recPtrTvShows (new RecordingsItemDir(tr("TV shows"), 1));
    RecordingsItemPtr recPtrMovieCollections (new RecordingsItemDir(tr("Movie collections"), 1));
// create "base" folders
    m_allRecordings.clear();
    if (scraperDataAvailable) {
      m_root->m_cmp_dir = RecordingsItemPtrCompare::ByReleaseDate;
      recPtrTvShows->m_s_release_date = "1";
      m_root->m_subdirs.push_back(recPtrTvShows);
      recPtrMovieCollections->m_s_release_date = "2";
      m_root->m_subdirs.push_back(recPtrMovieCollections);
      RecordingsItemPtr recPtrOthers (new RecordingsItemDir(tr("File system view"), 1));
      recPtrOthers->m_s_release_date = "3";
      m_rootFileSystem = recPtrOthers;
      m_root->m_subdirs.push_back(recPtrOthers);
    } else {
      m_rootFileSystem = m_root;
		}
		int idI = -1; // integer id
// add all recordings
#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_READ;
		for (cRecording* recording = (cRecording *)Recordings->First(); recording; recording = (cRecording *)Recordings->Next(recording)) {
		  idI = recording->Id();
#else
		for (cRecording* recording = Recordings.First(); recording; recording = Recordings.Next(recording)) {
		  idI++;
#endif
                  if (scraperDataAvailable) m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() + 1);
                  else m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() );

                  RecordingsItemPtr dir = m_rootFileSystem;
                  std::string name(recording->Name());

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
                      std::string recName(name.substr(index, name.length() - index));
                      RecordingsItemPtr recPtr (new RecordingsItemRec(idI, recMan->Md5Hash(recording), recName, recording));
                      dir->m_entries.push_back(recPtr);
		      m_allRecordings.push_back(recPtr);
                      // esyslog("live: DH: added rec: '%s'", recName.c_str());
                      if (scraperDataAvailable) {
                        switch (recPtr->scraperVideoType() ) {
                          case tMovie:
                            if (recPtr->m_s_collection_id == 0) break;
                            dir = recPtrMovieCollections->addDirCollectionIfNotExists(recPtr->m_s_collection_id, recording);
                            dir->m_entries.push_back(recPtr);
                            break;
                          case tSeries:
                            dir = recPtrTvShows->addDirIfNotExists(recPtr->scraperName() );
                            dir->setTvShow(recording);
                            if (recPtr->scraperSeasonNumber() != 0 || recPtr->scraperEpisodeNumber() != 0) {
                              dir = dir->addDirSeasonIfNotExists(recPtr->scraperSeasonNumber(), recording);
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
		// esyslog("live: DH: ------ RecordingsTree::RecordingsTree() --------");
	}

	RecordingsTree::~RecordingsTree()
	{
		// esyslog("live: DH: ****** RecordingsTree::~RecordingsTree() ********");
	}
	const std::vector<RecordingsItemPtr> *RecordingsTree::allRecordings(eSortOrder sortOrder) {
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

// icon with recording errors, and tooltip
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
    result.append(std::to_string(recordingErrors));
  }
  result.append("\" width = \"16px\"/> </div>");
  return result;
#else
  return "";
#endif
}

// find duplicates
bool ByScraperDataAvailable(const RecordingsItemPtr &first, tvType videoType) {
  return (int)first->scraperVideoType() < (int)videoType; // tNone if no scraper data. Move these to the end, by using <
}

void addDuplicateRecordingsNoSd(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where NO scraper data are available.
  const std::vector<RecordingsItemPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
// sorting for duplicatesLanguage is OK, even if language is not required here (language is ignored later)
  std::vector<RecordingsItemPtr>::const_iterator currentRecItem, recIterUpName, recIterUpShortText, recIterLowShortText;
  bool isSeries;

// see https://www.fluentcpp.com/2017/08/01/overloaded-functions-stl/
// static_cast<bool(*)(const RecordingsItemPtr &first, int videoType)>(f));
// #define RETURNS(...) noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__){ return __VA_ARGS__; }
// #define LIFT(f) [](auto&&... xs) RETURNS(f(::std::forward<decltype(xs)>(xs)...))
// std::for_each(begin(numbers), end(numbers), LIFT(f));
  for (currentRecItem = std::lower_bound(recItems->begin(), recItems->end(), tNone, ByScraperDataAvailable);
        currentRecItem != recItems->end(); ) {
//    if ( (*currentRecItem)->scraperDataAvailable() ) { currentRecItem++; continue;}
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

void addDuplicateRecordingsLang(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where scraper data are available.
// add only recordings with different language
  const std::vector<RecordingsItemPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemPtr>::const_iterator currentRecItem, nextRecItem, recIterUpName, recIterLowName;

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
void addDuplicateRecordingsSd(std::vector<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
// add duplicate recordings where scraper data are available.
// recordings with different languages are NOT duplicates
  const std::vector<RecordingsItemPtr> *recItems = RecordingsTree->allRecordings(eSortOrder::duplicatesLanguage);
  std::vector<RecordingsItemPtr>::const_iterator currentRecItem, recIterUpName, recIterLowName;

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
