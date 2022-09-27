
#include "recman.h"
#include "tools.h"
#include "services.h"
#include "epg_events.h"


// STL headers need to be before VDR tools.h (included by <vdr/videodir.h>)
#include <fstream>
#include <stack>
#include <algorithm>

#include <vdr/videodir.h>

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

  int RecordingsItemPtrCompare::FindBestMatch(RecordingsItemPtr & BestMatch, const std::list<RecordingsItemPtr>::iterator & First, const std::list<RecordingsItemPtr>::iterator & Last, const RecordingsItemPtr & EPG_Entry){
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
   std::list<RecordingsItemPtr>::iterator bestMatchIter;
   for ( std::list<RecordingsItemPtr>::iterator iter = First; iter != Last; ++iter)
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

	bool RecordingsItemPtrCompare::ByDescendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second)
	{
		return (first->StartTime() >= second->StartTime());
	}

	bool RecordingsItemPtrCompare::ByAscendingName(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           return first->NameForSearch().compare(second->NameForSearch() ) < 0;
	}

	bool RecordingsItemPtrCompare::ByDuplicatesName(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           return first->orderDuplicates(second, false);
	}

	bool RecordingsItemPtrCompare::ByDuplicates(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
          return first->orderDuplicates(second, true);
	}

	bool RecordingsItemPtrCompare::ByAscendingNameDesc(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           int i = first->NameForSearch().compare(second->NameForSearch() );
           if(i != 0) return i < 0;
           return first->CompareStD(second) < 0;
	}

	bool RecordingsItemPtrCompare::ByAscendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
// used for sort
	{
           int i = first->NameForSort().compare(second->NameForSort() );
           if(i != 0) return i < 0;
           return first->CompareStD(second) < 0;
	}

	bool RecordingsItemPtrCompare::ByDescendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first > second
// used for sort
	{
           return RecordingsItemPtrCompare::ByAscendingNameDescSort(second, first);
	}
        bool RecordingsItemPtrCompare::ByDescendingRecordingErrors(const RecordingsItemPtr & first, const RecordingsItemPtr & second){
           return first->RecordingErrors() >= second->RecordingErrors();
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

        std::string RecordingsItemPtrCompare::getNameForSort(const std::string &Name){
// remove punctuation characters at the beginning of the string
            unsigned int start;
            for(start = 0; start < Name.length() && ispunct( Name[ start ] ); start++ );
            return g_collate_char.transform(Name.data()+start, Name.data()+Name.length());
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


	/**
	 *  Implementation of class RecordingsItem:
	 */
	RecordingsItem::RecordingsItem(std::string const & name) :
		m_name(name),
                m_name_for_sort(RecordingsItemPtrCompare::getNameForSort(name)),
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
	  for (const auto &item:m_subdirs) result += item->numberOfRecordings();
	  return result;
        }

        RecordingsItemPtr RecordingsItem::addDirIfNotExists(const std::string &dirName) {
          RecordingDirsMap::iterator iter = m_subdirs.find(dirName);
          if (iter != m_subdirs.end() ) return *iter;
	  RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, Level() + 1));
	  m_subdirs.insert(recPtr);
          return recPtr;
	}

        RecordingsItemPtr RecordingsItem::addDirCollectionIfNotExists(int collectionId, const cRecording* recording) {
          RecordingDirsMap::iterator iter = m_subdirs.find(collectionId);
          if (iter != m_subdirs.end() ) return *iter;
	  RecordingsItemPtr recPtr2 (new RecordingsItemDirCollection(Level() + 1, recording));
	  m_subdirs.insert(recPtr2);
          return recPtr2;
	}

	bool RecordingsItem::addSubdirs(std::list<RecordingsItemPtr> &recList)
	{
// return sorted
	  for (const auto &subdir:m_subdirs) recList.push_back(subdir);
          if (!m_cmp_dir) return false;
          recList.sort(m_cmp_dir);
	  return true;
	}

	bool RecordingsItem::addRecordings(std::list<RecordingsItemPtr> &recList)
	{
// return sorted
	  for (const auto &rec:m_entries) recList.push_back(rec);
          if (!m_cmp_rec) return false;
          recList.sort(m_cmp_rec);
	  return true;
	}

	std::list<RecordingsItemPtr> RecordingsItem::getSubdirs(bool &sorted)
	{
	  std::list<RecordingsItemPtr> result;
          sorted = addSubdirs(result);
          return result;
	}

	std::list<RecordingsItemPtr> RecordingsItem::getRecordings(bool &sorted)
	{
	  std::list<RecordingsItemPtr> result;
          sorted = addRecordings(result);
          return result;
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

	void RecordingsItem::setSeason(const cRecording* recording) {
          if (m_cmp_rec) return;
          m_cmp_rec = RecordingsItemPtrCompare::ByEpisode;
          getScraperData(recording, cImageLevels(eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection) );
	}

	void RecordingsItem::setTvShow(const cRecording* recording) {
          if (m_cmp_dir) return;
          m_cmp_dir = RecordingsItemPtrCompare::BySeason;
          getScraperData(recording, cImageLevels(eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection) );
	}

	void RecordingsItem::getScraperData(const cRecording* recording, const cImageLevels &imageLevels, std::string *collectionName) {
          cGetScraperOverview scraperOverview (NULL, recording, &m_s_title, &m_s_episode_name, &m_s_IMDB_ID, &m_s_image, imageLevels,
            cOrientations(eOrientation::landscape, eOrientation::portrait, eOrientation::banner),
            &m_s_release_date, collectionName);
          if (scraperOverview.call(LiveSetup().GetPluginScraper()) ) {
            m_s_videoType = scraperOverview.m_videoType;
            if (!scraperDataAvailable() ) return;
            m_s_dbid = scraperOverview.m_dbid;
            m_s_runtime = scraperOverview.m_runtime;
            m_s_collection_id = scraperOverview.m_collectionId;
            m_s_episode_number = scraperOverview.m_episodeNumber;
            m_s_season_number = scraperOverview.m_seasonNumber;
          } else {
// service "GetScraperOverview" is not available, just get the image
            m_s_videoType = eVideoType::none;
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

        bool RecordingsItem::orderDuplicates(const RecordingsItemPtr &second, bool alwaysShortText) const {
// this is a < operation. Return false in case of ==   !!!!!
          if (m_s_videoType != second->m_s_videoType) return (int)m_s_videoType < (int)second->m_s_videoType;
          switch (m_s_videoType) {
            case eVideoType::movie:
              return m_s_dbid < second->m_s_dbid;
            case eVideoType::tvShow:
              if (m_s_dbid != second->m_s_dbid) return m_s_dbid < second->m_s_dbid;
              if (m_s_season_number != second->m_s_season_number) return m_s_season_number < second->m_s_season_number;
              if (m_s_episode_number != second->m_s_episode_number) return m_s_episode_number < second->m_s_episode_number;
              if (m_s_season_number != 0 || m_s_episode_number != 0) return false;  // they are equal => < operator results in false ...
              return CompareTexts(second) < 0;
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
          m_name_for_sort = RecordingsItemPtrCompare::getNameForSort(m_name);
	}

	RecordingsItemDirCollection::~RecordingsItemDirCollection() { }

	/**
	 *  Implementation of class RecordingsItemRec:
	 */
	RecordingsItemRec::RecordingsItemRec(const std::string& id, const std::string& name, const cRecording* recording) :
		RecordingsItem(name),
		m_recording(recording),
		m_id(id),
                m_isArchived(RecordingsManager::GetArchiveType(m_recording) ),
                m_duration(m_recording->FileName() ? m_recording->LengthInSeconds() / 60 : 0)
	{
          // dsyslog("live: REC: C: rec %s -> %s", name.c_str(), parent->Name().c_str());
          getScraperData(recording,
            cImageLevels(eImageLevel::episodeMovie, eImageLevel::seasonMovie, eImageLevel::tvShowCollection, eImageLevel::anySeasonCollection));
	}

	RecordingsItemRec::~RecordingsItemRec()
	{
		// dsyslog("live: REC: D: rec %s", Name().c_str());
	}

        void RecordingsItemRec::AppendHint(std::string &target) const
        {
           if (RecInfo()->ShortText()   ) {
             AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->ShortText() );
             target.append("&lt;br /&gt;");
           }
           else if (RecInfo()->Description() ) {
             AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->Description() );
             target.append("&lt;br /&gt;");
           }
           AppendHtmlEscaped(target, tr("Click to view details.")   );
        }

        void RecordingsItemRec::AppendShortTextOrDesc(std::string &target) const
        {
          const char *text = ShortText();
          if (!text || Name() == text ) text = RecInfo()->Description();
// Spielfilm Italien / Großbritannien / USA 1965 (Doctor Zhivago)
          if (!text) return;
          if ((int)strlen(text) < 70)
            AppendHtmlEscapedAndCorrectNonUTF8(target, text);
          else {
            const char *end = text + 69;
            for (; *end && *end != ' '; end++);
            AppendHtmlEscapedAndCorrectNonUTF8(target, std::string(text, end-text).c_str() );
            target.append("...");
          }
        }
        const char *RecordingsItemRec::RecordingErrorsIcon() const
        {
           if (RecordingErrors() == 0) return "NoRecordingErrors.png";
           if (RecordingErrors()  > 0) return "RecordingErrors.png";
           return "NotCheckedForRecordingErrors.png";
        }

        void RecordingsItemRec::AppendRecordingErrorsStr(std::string &target) const
        {
          if (RecordingErrors() == 0) AppendHtmlEscaped(target, tr("No recording errors"));
          if (RecordingErrors()  > 0) {
            AppendHtmlEscaped(target, tr("Number of recording errors:"));
            target.append(" ");
            target.append(std::to_string(RecordingErrors() ));
           }
          if (RecordingErrors() < 0) AppendHtmlEscaped(target, tr("Recording errors unknown"));
        }

        const int RecordingsItemRec::SD_HD()
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

        void RecordingsItemRec::AppendIMDb(std::string &target) const {
          if (LiveSetup().GetShowIMDb()) { 
            if (m_s_IMDB_ID.empty() ) {
              target.append("<a href=\"http://www.imdb.com/find?s=all&q=");
              AppendHtmlEscaped( target, StringUrlEncode(Name() ).c_str() );
            } else {
              target.append("<a href=\"https://www.imdb.com/title/");
              target.append(m_s_IMDB_ID);
            }
            target.append("\" target=\"_blank\"><img src=\"");
            target.append(LiveSetup().GetThemedLinkPrefixImg() );
            target.append("imdb.png\" title=\"");
            AppendHtmlEscaped( target, tr("Find more at the Internet Movie Database.") );
            target.append("\"/></a>\n");
          }
        }

        void RecordingsItemRec::AppendRecordingAction(std::string &target, const char *A, const char *Img, const char *Title, const std::string argList){
          target.append("<a href=\"");
          target.append(A);
          target.append(Id() );
          target.append(argList);
          target.append("\" title=\"");
          AppendHtmlEscaped( target, tr(Title) );
          target.append("\"><img src=\"");
          target.append(LiveSetup().GetThemedLinkPrefixImg() );
          target.append(Img);
          target.append("\" /></a>\n");
        }

        void RecordingsItemRec::AppendasJSArray(std::string &target, bool displayFolder, const std::string argList, int level){
// list item, classes, space depending on level
//          target.append("<script>Recordings([");
          target.append("[");
// [0] : Level
          if(displayFolder) {
            target.append("0,\"");
          } else {
// add some space
            target.append( std::to_string(level) );
            target.append(",\"");
          }
// [1] : ID
          target.append(Id().c_str() + 10);
          target.append("\",\"");
// [2] : Archived
          if (IsArchived() ) {
            target.append("<img src=\"");
            target.append(LiveSetup().GetThemedLinkPrefixImg() );
            target.append("on_dvd.png\" alt=\"on_dvd\" title=\"");
            AppendHtmlEscaped(target, ArchiveDescr().c_str() );
            target.append("/>");
          }
          target.append("\", \"");
// scraper data
// [3] : image.path  (nach "/tvscraper/")
          target.append(m_s_image.path);
          target.append("\", \"");
// [4] : "pt" if m_s_image.width <= m_s_image.height, otherwise= ""
          if (m_s_image.width <= m_s_image.height) target.append("pt");
          target.append("\", \"");
          if (!scraperDataAvailable() ) {
            m_s_title = "";
            m_s_videoType = eVideoType::none;
            m_s_runtime = 0;
            m_s_release_date = "";
          }
// [5] : title (scraper)
          AppendHtmlEscapedAndCorrectNonUTF8(target, m_s_title.c_str() );
          target.append("\", \"");
          if (m_s_videoType == eVideoType::tvShow && (m_s_episode_number != 0 || m_s_season_number != 0)) {
// [6] : season/episode/episode name (scraper)
            target.append(std::to_string(m_s_season_number));
            target.append("E");
            target.append(std::to_string(m_s_episode_number));
            target.append(" ");
            AppendHtmlEscapedAndCorrectNonUTF8(target, m_s_episode_name.c_str() );
          }
          target.append("\", \"");
          if (m_s_runtime) {
// [7] : runtime (scraper)
            AppendDuration(target, tr("(%d:%02d)"), m_s_runtime / 60, m_s_runtime % 60);
          }
          target.append("\", \"");
          if (!m_s_release_date.empty() ) {
// [8] : relase date (scraper)
            target.append(m_s_release_date);
          }
          target.append("\", \"");
// recording_spec: Day, time & duration
// [9] : recording_spec: Day, time & duration
          AppendDateTime(target, tr("%a,"), StartTime());  // day of week
          target.append(" ");
	  AppendDateTime(target, tr("%b %d %y"), StartTime());  // date
          target.append(" ");
	  AppendDateTime(target, tr("%I:%M %p"), StartTime() );  // time
          if(Duration() >= 0) {
            target.append(" ");
            AppendDuration(target, tr("(%d:%02d)"), Duration() / 60, Duration() % 60);
          }
          target.append("\", ");
// RecordingErrors, Icon
#if VDRVERSNUM >= 20505
// [10] : Number of recording errors
          target.append(std::to_string(RecordingErrors() ));
#else
          target.append("-100");
#endif
          target.append(", \"");
// [11] HD_SD
          target.append(SD_HD() == 0 ? "s": "h");
          target.append("\", \"");
// [12] channel name
          AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->ChannelName() );
          target.append("\", \"");
// [13] NewR()
          target.append(NewR() );
          target.append("\", \"");
// [14] Short text / descr
          AppendShortTextOrDesc(target);
          target.append("\", \"");
// [15] Name
          AppendHtmlEscaped(target, Name().c_str() );
          target.append("\", \"");
// [16] Path / folder
          if( *(const char *)Recording()->Folder() && displayFolder) {
             target.append(" (");
             AppendHtmlEscaped(target, (const char *)Recording()->Folder() );
             target.append(")");
          }
          target.append("\", \"");
// recording_actions
// [17] Arglist
          if (!IsArchived()) target.append(argList);
          target.append("\", \"");
// [18]  IMDB ID
          target.append(m_s_IMDB_ID);
          target.append("\", \"");
// [19] ArchiveDescr()
          if (IsArchived()) AppendHtmlEscapedAndCorrectNonUTF8(target, ArchiveDescr().c_str() );
          target.append("\"]");
//          target.append("])</script>");
        }

        void RecordingsItemRec::AppendasHtml(std::string &target, bool displayFolder, const std::string argList, int level){
// list item, classes, space depending on level
          target.append("<li class=\"recording\"><div class=\"recording_item\"><div class=\"recording_imgs\">");
          if(!displayFolder) {
// add some space
            target.append("<img src=\"img/transparent.png\" width=\"");
            target.append( std::to_string(16 * level ) );
            target.append("px\" height=\"16px\" />\n");
          }
          if (IsArchived() ) {
            target.append("<img src=\"");
            target.append(LiveSetup().GetThemedLinkPrefixImg() );
            target.append("on_dvd.png\" alt=\"on_dvd\" title=\"");
            AppendHtmlEscaped(target, ArchiveDescr().c_str() );
            target.append("/>");
          } else {
#if TNTVERSION >= 30000
            target.append("<input type=\"checkbox\" name=\"deletions[]\" value=\"");
#else
            target.append("<input type=\"checkbox\" name=\"deletions\" value=\"");
#endif
            target.append(Id());
            target.append("\" />" );
          }
// scraper data
          if (!LiveSetup().GetTvscraperImageDir().empty() ) {
            target.append("</div>\n<div class=\"thumb\">");
            target.append("<a class=\"thumb\" href=\"epginfo.html?epgid=");
            target.append(Id() );
            target.append("\"><img src=\"" );
            if (!m_s_image.path.empty() ) {  
              target.append("/tvscraper/");
              target.append(m_s_image.path);
              if (m_s_image.width > m_s_image.height) target.append("\" class=\"thumb");
              else target.append("\" class=\"thumbpt");
            } else {
              target.append("img/transparent.png\" height=\"16px");
            }
            if (scraperDataAvailable() ) {
              target.append("\" title=\"");
              AppendHtmlEscapedAndCorrectNonUTF8(target, m_s_title.c_str() );
              if (m_s_videoType == eVideoType::tvShow && (m_s_episode_number != 0 || m_s_season_number != 0)) {
                target.append("<br/>S");
                target.append(std::to_string(m_s_season_number));
                target.append("E");
                target.append(std::to_string(m_s_episode_number));
                target.append(" ");
                AppendHtmlEscapedAndCorrectNonUTF8(target, m_s_episode_name.c_str() );
              }
              if (m_s_runtime) {
                target.append("<br/>");
                AppendDuration(target, tr("(%d:%02d)"), m_s_runtime / 60, m_s_runtime % 60);
              }
              if (!m_s_release_date.empty() ) {
                target.append("<br/>");
                target.append(m_s_release_date);
              }
            }
            target.append("\" /> </a>");
          }
// recording_spec: Day, time & duration
          target.append("</div>\n<div class=\"recording_spec\"><div class=\"recording_day\">");
          AppendDateTime(target, tr("%a,"), StartTime());  // day of week
          target.append(" ");
	  AppendDateTime(target, tr("%b %d %y"), StartTime());  // date
          target.append(" ");
	  AppendDateTime(target, tr("%I:%M %p"), StartTime() );  // time
          if(Duration() >= 0) {
            target.append(" ");
            AppendDuration(target, tr("(%d:%02d)"), Duration() / 60, Duration() % 60);
          }
          target.append("</div>");
// RecordingErrors, Icon
#if VDRVERSNUM >= 20505
          target.append("<div class=\"recording_errors\"><img src=\"");
          target.append(LiveSetup().GetThemedLinkPrefixImg() );
          target.append(RecordingErrorsIcon() );
          target.append("\" width = \"16px\" title=\"");
// RecordingErrors, Tooltip
          AppendRecordingErrorsStr(target);
          target.append("\" /> </div>");
#endif
// HD_SD, with channel name
          target.append("<div class=\"recording_sd_hd\"><img src=\"");
          target.append(LiveSetup().GetThemedLinkPrefixImg() );
          target.append( SD_HD_icon() );
          target.append("\" width = \"25px\" title=\"");
          AppendHtmlEscapedAndCorrectNonUTF8(target, RecInfo()->ChannelName() );
          target.append("\" /> </div>\n");
// Recording name
          target.append("<div class=\"recording_name");
          target.append(NewR() );
          target.append("\"><a title=\"");
          AppendHint(target);
          target.append("\" href=\"epginfo.html?epgid=");
          target.append(Id() );
          target.append("\">" );
          AppendHtmlEscaped(target, Name().c_str() );
//          target.append(" (" );
//          AppendHtmlEscaped(target, NameForSearch().c_str() );
//          target.append(")" );
// Path / folder
          if( *(const char *)Recording()->Folder() && displayFolder) {
             target.append(" (");
             AppendHtmlEscaped(target, (const char *)Recording()->Folder() );
             target.append(")");
          }
          target.append("<br /><span>");
// second line of recording name
          if(ShortText() && Name() != ShortText() ) 
             AppendHtmlEscapedAndCorrectNonUTF8(target, ShortText() );
                     // Tntnet30 throw: tntnet.worker - http-Error: 500 character conversion failed
          else 
             target.append("&nbsp;");
// recording_actions
          target.append("</span></a></div></div>\n<div class=\"recording_actions\">");
          if (!IsArchived()) {
            AppendRecordingAction(target, "vdr_request/play_recording?param=", "play.png", "play this recording", argList);
            AppendRecordingAction(target, "playlist.m3u?recid=", "playlist.png", "Stream this recording into media player.", argList);
            AppendIMDb(target);
            AppendRecordingAction(target, "edit_recording.html?recid=", "edit.png", "Edit recording", argList);
            AppendRecordingAction(target, "recordings.html?todel=", "del.png", "Delete this recording from hard disc!", argList);

          } else {
            target.append("<img src=\"img/transparent.png\" width=\"16px\" height=\"16px\" />");
            AppendIMDb(target);
          }
          target.append("</div>");
          if (IsArchived()) {
            target.append("<div class=\"recording_arch\">");
            AppendHtmlEscaped(target,  ArchiveDescr().c_str() );
            target.append("</div>");
          }
          target.append("</div></li>");
        }

	/**
	 * Implemetation of class RecordingsItemDummy
	 */
        RecordingsItemDummy::RecordingsItemDummy(const std::string &Name, const std::string &ShortText, const std::string &Description, long Duration, cGetScraperOverview *scraperOverview):
                RecordingsItem(Name),
                m_short_text(ShortText.c_str() ),
                m_description(Description.c_str() ),
                m_duration( Duration / 60 )
                {
                  if (scraperOverview) {
                    m_s_videoType = scraperOverview->m_videoType;
                    m_s_dbid = scraperOverview->m_dbid;
                    m_s_episode_number = scraperOverview->m_episodeNumber;
                    m_s_season_number = scraperOverview->m_seasonNumber;
                  } else {
                    m_s_videoType = eVideoType::none;
                  }
                }

  bool operator< (const RecordingsItemPtr &a, const RecordingsItemPtr &b) { return *a < b; }
  bool operator< (const std::string &a, const RecordingsItemPtr &b) { return a < b->Name(); }
  bool operator< (const RecordingsItemPtr &a, const std::string &b) { return a->Name() < b; }
  bool operator< (int a, const RecordingsItemPtr &b) { return a < b->scraperCollectionId(); }
  bool operator< (const RecordingsItemPtr &a, int b) { return a->scraperCollectionId() < b; }

	/**
	 *  Implementation of class RecordingsTree:
	 */
	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir("", 0))
	{
		// esyslog("live: DH: ****** RecordingsTree::RecordingsTree() ********");
// check availability of scraper data
		cGetScraperOverview scraperOverview;
		bool scraperDataAvailable = scraperOverview.call(LiveSetup().GetPluginScraper());
                RecordingsItemPtr recPtrTvShows (new RecordingsItemDir(tr("TV shows"), 1));
                RecordingsItemPtr recPtrMovieCollections (new RecordingsItemDir(tr("Movie collections"), 1));
                if (scraperDataAvailable) {
// create "base" folders
                  m_root->m_cmp_dir = RecordingsItemPtrCompare::ByReleaseDate;
                  recPtrTvShows->m_s_release_date = "1";
                  m_root->m_subdirs.insert(recPtrTvShows);
                  recPtrMovieCollections->m_s_release_date = "2";
                  m_root->m_subdirs.insert(recPtrMovieCollections);
                  RecordingsItemPtr recPtrOthers (new RecordingsItemDir(tr("File system view"), 1));
                  recPtrOthers->m_s_release_date = "3";
                  m_rootFileSystem = recPtrOthers;
                  m_root->m_subdirs.insert(recPtrOthers);
                } else {
                  m_rootFileSystem = m_root;
		}
// add all recordings
#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_READ;
		for (cRecording* recording = (cRecording *)Recordings->First(); recording; recording = (cRecording *)Recordings->Next(recording)) {
#else
		for (cRecording* recording = Recordings.First(); recording; recording = Recordings.Next(recording)) {
#endif
                  if (scraperDataAvailable) m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() + 1);
                  else m_maxLevel = std::max(m_maxLevel, recording->HierarchyLevels() );

                  RecordingsItemPtr dir = m_rootFileSystem;
                  std::string name(recording->Name());

                  // esyslog("live: DH: recName = '%s'", recording->Name());
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
                      RecordingsItemPtr recPtr (new RecordingsItemRec(recMan->Md5Hash(recording), recName, recording));
                      dir->m_entries.insert(recPtr);
                      // esyslog("live: DH: added rec: '%s'", recName.c_str());
                      if (scraperDataAvailable) {
                        switch (recPtr->scraperVideoType() ) {
                          case eVideoType::movie:
                            if (recPtr->m_s_collection_id == 0) break;
                            dir = recPtrMovieCollections->addDirCollectionIfNotExists(recPtr->m_s_collection_id, recording);
                            dir->m_entries.insert(recPtr);
                            break;
                          case eVideoType::tvShow:
                            dir = recPtrTvShows->addDirIfNotExists(recPtr->scraperName() );
                            dir->setTvShow(recording);
                            if (recPtr->scraperSeasonNumber() != 0 || recPtr->scraperEpisodeNumber() != 0) {
                              dir = dir->addDirIfNotExists(std::to_string(recPtr->scraperSeasonNumber() ));
                              dir->setSeason(recording);
                            }
                            dir->m_entries.insert(recPtr);
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
		// esyslog("live: DH: ------ RecordingsTree::RecordingsTree() --------");
	}

	RecordingsTree::~RecordingsTree()
	{
		// esyslog("live: DH: ****** RecordingsTree::~RecordingsTree() ********");
	}

        void RecordingsTree::addAllRecordings(std::list<RecordingsItemPtr> &recList, RecordingsItemPtr dir)
	{
	  for (const auto &subdir:dir->m_subdirs) addAllRecordings(recList, subdir);
          dir->addRecordings(recList);
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

void addAllDuplicateRecordings(std::list<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
  std::list<RecordingsItemPtr> recItems;
  std::list<RecordingsItemPtr>::iterator currentRecItem, recIterUpName, recIterLowName, recIterUpShortText, recIterLowShortText;
  int numberOfRecordingsWithThisName;
  bool isSeries;

  RecordingsTree->addAllRecordings(recItems);
  recItems.sort(RecordingsItemPtrCompare::ByDuplicates);

  for (currentRecItem = recItems.begin(); currentRecItem != recItems.end(); ) {
    recIterLowName = currentRecItem;
    recIterUpName  = std::upper_bound (currentRecItem , recItems.end(), *currentRecItem, RecordingsItemPtrCompare::ByDuplicatesName);

    currentRecItem++;
    if (recIterLowName == recIterUpName ) continue; // there is no recording with this name (internal error)
    if (currentRecItem == recIterUpName ) continue; // there is only one recording with this name
    if ( (*recIterLowName)->scraperDataAvailable() ) {
      isSeries = false;  // no special for series required
    } else {
      for( numberOfRecordingsWithThisName = 1; currentRecItem != recIterUpName; currentRecItem++, numberOfRecordingsWithThisName++);
      if (numberOfRecordingsWithThisName > 5) isSeries = true; else isSeries = false;
    }
    if (isSeries) {
      for (currentRecItem = recIterLowName; currentRecItem != recIterUpName;){
        recIterLowShortText = currentRecItem;
        recIterUpShortText  = std::upper_bound (currentRecItem , recIterUpName, *currentRecItem, RecordingsItemPtrCompare::ByDuplicates);
        currentRecItem++;
        if (currentRecItem == recIterUpShortText ) continue; // there is only one recording with this short text
        for(currentRecItem = recIterLowShortText; currentRecItem != recIterUpShortText; currentRecItem++)
          DuplicateRecItems.push_back(*currentRecItem);
      }
    } else { // not a series
      for(currentRecItem = recIterLowName; currentRecItem != recIterUpName; currentRecItem++)
        DuplicateRecItems.push_back(*currentRecItem);
    }
  }
}

} // namespace vdrlive
