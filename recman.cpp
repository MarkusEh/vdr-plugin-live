
#include "recman.h"
#include "tools.h"

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
	std::shared_ptr<RecordingsList> RecordingsManager::m_recList;
	std::shared_ptr<DirectoryList> RecordingsManager::m_recDirs;
#if VDRVERSNUM >= 20301
	cStateKey RecordingsManager::m_recordingsStateKey;
#else
	int RecordingsManager::m_recordingsState = 0;
#endif

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

	RecordingsListPtr RecordingsManager::GetRecordingsList(bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, std::shared_ptr<RecordingsList>());
		}
		return RecordingsListPtr(recMan, std::shared_ptr<RecordingsList>(new RecordingsList(m_recList, ascending)));
	}

	RecordingsListPtr RecordingsManager::GetRecordingsList(time_t begin, time_t end, bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, std::shared_ptr<RecordingsList>());
		}
		return RecordingsListPtr(recMan, std::shared_ptr<RecordingsList>(new RecordingsList(m_recList, ascending)));
	}

	DirectoryListPtr RecordingsManager::GetDirectoryList() const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (!recMan) {
			return DirectoryListPtr(recMan, std::shared_ptr<DirectoryList>());
		}
		return DirectoryListPtr(recMan, m_recDirs);
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

		// will return != 0 only, if the Recordings List has been changed since last read
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
			return RecordingsManagerPtr();
		}

		// StateChanged must be executed every time, so not part of
		// the short cut evaluation in the if statement below.
#if VDRVERSNUM >= 20301
		bool stateChanged = StateChanged();
#else
		bool stateChanged = Recordings.StateChanged(m_recordingsState);
#endif
		if (stateChanged || (!m_recTree) || (!m_recList) || (!m_recDirs)) {
			if (stateChanged) {
				m_recTree.reset();
				m_recList.reset();
				m_recDirs.reset();
			}
			if (stateChanged || !m_recTree) {
				m_recTree = std::shared_ptr<RecordingsTree>(new RecordingsTree(recMan));
			}
			if (!m_recTree) {
				esyslog("live: creation of recordings tree failed!");
				return RecordingsManagerPtr();
			}
			if (stateChanged || !m_recList) {
				m_recList = std::shared_ptr<RecordingsList>(new RecordingsList(RecordingsTreePtr(recMan, m_recTree)));
			}
			if (!m_recList) {
				esyslog("live: creation of recordings list failed!");
				return RecordingsManagerPtr();
			}
			if (stateChanged || !m_recDirs) {
				m_recDirs = std::shared_ptr<DirectoryList>(new DirectoryList(recMan));
			}
			if (!m_recDirs) {
				esyslog("live: creation of directory list failed!");
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

        int RecordingsItemPtrCompare::Compare2(int &numEqualChars, const RecordingsItemPtr & first, const RecordingsItemPtr & second) {
// compare short text & description
          numEqualChars = 0;
          ShortTextDescription chfirst (first ->ShortText() , first ->Description() );
          ShortTextDescription chsecond(second->ShortText() , second->Description() );
          wint_t flc;
          wint_t slc;
          do {
            flc = chfirst.getNextNonPunctChar();
            slc = chsecond.getNextNonPunctChar();
            if ( flc < slc ) return -1;
            if ( flc > slc ) return  1;
            ++numEqualChars;
          } while ( flc && slc );
          --numEqualChars;
          return 0;
        }

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


	/**
	 * Implemetation of class RecordingsItemPtrCompare
	 */
	bool RecordingsItemPtrCompare::ByAscendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second)
	{
		return (first->StartTime() < second->StartTime());
	}

	bool RecordingsItemPtrCompare::ByDescendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second)
	{
		return (first->StartTime() >= second->StartTime());
	}

	int RecordingsItemPtrCompare::Compare(int &numEqualChars, const RecordingsItemPtr &first, const RecordingsItemPtr &second)
	{
                numEqualChars = 0;
                int i = first->NameForSearch().compare(second->NameForSearch() );
                if(i != 0) return i;
             // name is identical, compare short text
                i = RecordingsItemPtrCompare::compareLC(numEqualChars, first->ShortText(), second->ShortText() );
                if(i != 0) return i;
                i = RecordingsItemPtrCompare::compareLC(numEqualChars, first->Description(), second->Description() );
                return i;
	}

	bool RecordingsItemPtrCompare::ByAscendingName(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           return first->NameForSearch().compare(second->NameForSearch() ) < 0;
	}

	bool RecordingsItemPtrCompare::ByAscendingNameShortText(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
          int i = first->NameForSearch().compare(second->NameForSearch() );
          if(i != 0) return i < 0;

          return RecordingsItemPtrCompare::compareLC(i, first->ShortText(), second->ShortText() ) < 0;
	}

	bool RecordingsItemPtrCompare::ByAscendingNameDesc(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
	{
           int i = first->NameForSearch().compare(second->NameForSearch() );
           if(i != 0) return i < 0;
           return RecordingsItemPtrCompare::Compare2(i, first, second) < 0;
	}

	bool RecordingsItemPtrCompare::ByAscendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first < second
// used for sort
	{
           int i = first->NameForSort().compare(second->NameForSort() );
           if(i != 0) return i < 0;
           return RecordingsItemPtrCompare::Compare2(i, first, second) < 0;
	}

	bool RecordingsItemPtrCompare::ByDescendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second)  // return first > second
// used for sort
	{
           return RecordingsItemPtrCompare::ByAscendingNameDescSort(second, first);
	}
        bool RecordingsItemPtrCompare::ByDescendingRecordingErrors(const RecordingsItemPtr & first, const RecordingsItemPtr & second){
           return first->RecordingErrors() >= second->RecordingErrors();
        }

        std::string RecordingsItemPtrCompare::getNameForSort(const std::string &Name){
// remove punctuation characters at the beginning of the string
            unsigned int start;
            for(start = 0; start < Name.length() && ispunct( Name[ start ] ); start++ );
            return g_collate_char.transform(Name.data()+start, Name.data()+Name.length());
        }

        int RecordingsItemPtrCompare::compareLC(int &numEqualChars, const char *first, const char *second) {
          bool fe = !first  || !first[0];    // is first  string empty string?
          bool se = !second || !second[0];   // is second string empty string?
          if (fe && se) return 0;
          if (se) return  1;
          if (fe) return -1;
// compare strings case-insensitive
          for(; *first && *second; ) {
//          if (*first == *second) { first++; second++; numEqualChars++; continue; }
            wint_t  flc = towlower(getNextUtfCodepoint(first));
            wint_t  slc = towlower(getNextUtfCodepoint(second));
            if ( flc < slc ) return -1;
            if ( flc > slc ) return  1;
            numEqualChars++;
          }
          if (*second ) return -1;
          if (*first  ) return  1;
          return 0;
        }


	/**
	 *  Implementation of class RecordingsItem:
	 */
	RecordingsItem::RecordingsItem(std::string const & name, RecordingsItemPtr parent) :
		m_level((parent != NULL) ? parent->Level() + 1 : 0),
		m_name(name),
                m_name_for_sort(RecordingsItemPtrCompare::getNameForSort(name)),
                m_name_for_search(RecordingsItem::GetNameForSearch(name)),
		m_entries(),
		m_parent(parent)
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


	/**
	 *  Implementation of class RecordingsItemDir:
	 */
	RecordingsItemDir::RecordingsItemDir(const std::string& name, int level, RecordingsItemPtr parent) :
		RecordingsItem(name, parent),
		m_level(level)
	{
		// dsyslog("live: REC: C: dir %s -> %s", name.c_str(), parent ? parent->Name().c_str() : "ROOT");
	}

	RecordingsItemDir::~RecordingsItemDir()
	{
		// dsyslog("live: REC: D: dir %s", Name().c_str());
	}


	/**
	 *  Implementation of class RecordingsItemRec:
	 */
	RecordingsItemRec::RecordingsItemRec(const std::string& id, const std::string& name, const cRecording* recording, RecordingsItemPtr parent) :
		RecordingsItem(name, parent),
		m_recording(recording),
		m_id(id),
                m_isArchived(RecordingsManager::GetArchiveType(m_recording) ),
                m_duration(m_recording->FileName() ? m_recording->LengthInSeconds() / 60 : 0)
	{
		// dsyslog("live: REC: C: rec %s -> %s", name.c_str(), parent->Name().c_str());
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
            target.append("<a href=\"http://www.imdb.com/find?s=all&q=");
            AppendHtmlEscaped( target, StringUrlEncode(Name() ).c_str() );
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

        void RecordingsItemRec::AppendasHtml(std::string &target, bool displayFolder, const std::string argList){
// list item, classes, space depending on level
          target.append("<li class=\"recording\"><div class=\"recording_item\"><div class=\"recording_imgs\">");
          if(!displayFolder) {
// add some space
            target.append("<img src=\"img/transparent.png\" width=\"");
            target.append( std::to_string(16 * Level() ) );
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
// recording_spec: Day, time & duration
          target.append("</div>\n<div class=\"recording_spec\"><div class=\"recording_day\">");
          AppendDateTime(target, tr("%a,"), StartTime());  // day of week
          target.append(" ");
	  AppendDateTime(target, tr("%b %d %y"), StartTime());  // date
          target.append(" ");
	  AppendDateTime(target, tr("%I:%M %p"), StartTime() );  // time
          target.append("</div><div class=\"recording_duration\">");
          if(Duration() >= 0) AppendDuration(target, tr("(%d:%02d)"), Duration() / 60, Duration() % 60);
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
        RecordingsItemDummy::RecordingsItemDummy(const std::string &Name, const std::string &ShortText, const std::string &Description, long Duration):
                RecordingsItem(Name, RecordingsItemPtr() ),
                m_short_text(ShortText.c_str() ),
                m_description(Description.c_str() ),
                m_duration( Duration / 60 )
                { }

	/**
	 *  Implementation of class RecordingsTree:
	 */
	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir("", 0, RecordingsItemPtr()))
	{
		// esyslog("live: DH: ****** RecordingsTree::RecordingsTree() ********");
#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_READ;
		for (cRecording* recording = (cRecording *)Recordings->First(); recording; recording = (cRecording *)Recordings->Next(recording)) {
#else
		for (cRecording* recording = Recordings.First(); recording; recording = Recordings.Next(recording)) {
#endif
			if (m_maxLevel < recording->HierarchyLevels()) {
				m_maxLevel = recording->HierarchyLevels();
			}

			RecordingsItemPtr dir = m_root;
			std::string name(recording->Name());

			// esyslog("live: DH: recName = '%s'", recording->Name());
			int level = 0;
			size_t index = 0;
			size_t pos = 0;
			do {
				pos = name.find('~', index);
				if (pos != std::string::npos) {
					std::string dirName(name.substr(index, pos - index));
					index = pos + 1;
					RecordingsMap::iterator i = findDir(dir, dirName);
					if (i == dir->m_entries.end()) {
						RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, level, dir));
						dir->m_entries.insert(std::pair<std::string, RecordingsItemPtr > (dirName, recPtr));
						i = findDir(dir, dirName);
#if 0
						if (i != dir->m_entries.end()) {
							// esyslog("live: DH: added dir: '%s'", dirName.c_str());
						}
						else {
							// esyslog("live: DH: panic: didn't found inserted dir: '%s'", dirName.c_str());
						}
#endif
					}
					dir = i->second;
					// esyslog("live: DH: current dir: '%s'", dir->Name().c_str());
					level++;
				}
				else {
					std::string recName(name.substr(index, name.length() - index));
					RecordingsItemPtr recPtr (new RecordingsItemRec(recMan->Md5Hash(recording), recName, recording, dir));
					dir->m_entries.insert(std::pair<std::string, RecordingsItemPtr> (recName, recPtr));
					// esyslog("live: DH: added rec: '%s'", recName.c_str());
				}
			} while (pos != std::string::npos);
		}
		// esyslog("live: DH: ------ RecordingsTree::RecordingsTree() --------");
	}

	RecordingsTree::~RecordingsTree()
	{
		// esyslog("live: DH: ****** RecordingsTree::~RecordingsTree() ********");
	}

	RecordingsMap::iterator RecordingsTree::begin(const std::vector<std::string>& path)
	{
		if (path.empty()) {
			return m_root->m_entries.begin();
		}

		RecordingsItemPtr recItem = m_root;
		for (std::vector<std::string>::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			std::pair<RecordingsMap::iterator, RecordingsMap::iterator> range = recItem->m_entries.equal_range(*i);
			for (RecordingsMap::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.begin();
	}

	RecordingsMap::iterator RecordingsTree::end(const std::vector<std::string>&path)
	{
		if (path.empty()) {
			return m_root->m_entries.end();
		}

		RecordingsItemPtr recItem = m_root;
		for (std::vector<std::string>::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			std::pair<RecordingsMap::iterator, RecordingsMap::iterator> range = recItem->m_entries.equal_range(*i);
			for (RecordingsMap::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.end();
	}

	RecordingsMap::iterator RecordingsTree::findDir(RecordingsItemPtr& dir, const std::string& dirName)
	{
		std::pair<RecordingsMap::iterator, RecordingsMap::iterator> range = dir->m_entries.equal_range(dirName);
		for (RecordingsMap::iterator i = range.first; i != range.second; ++i) {
			if (i->second->IsDir()) {
				return i;
			}
		}
		return dir->m_entries.end();
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
	 *  Implementation of class RecordingsList:
	 */
	RecordingsList::RecordingsList(RecordingsTreePtr recTree) :
		m_pRecVec(new RecVecType())
	{
		if (!m_pRecVec) {
			return;
		}

		std::stack<RecordingsItemPtr> treeStack;
		treeStack.push(recTree->Root());

		while (!treeStack.empty()) {
			RecordingsItemPtr current = treeStack.top();
			treeStack.pop();
			for (RecordingsMap::const_iterator iter = current->begin(); iter != current->end(); ++iter) {
				RecordingsItemPtr recItem = iter->second;
				if (recItem->IsDir()) {
					treeStack.push(recItem);
				}
				else {
					m_pRecVec->push_back(recItem);
				}
			}
		}
	}

	RecordingsList::RecordingsList(std::shared_ptr<RecordingsList> recList, bool ascending) :
		m_pRecVec(new RecVecType(recList->size()))
	{
		if (!m_pRecVec) {
			return;
		}
		if (ascending) {
			partial_sort_copy(recList->begin(), recList->end(), m_pRecVec->begin(), m_pRecVec->end(), Ascending());
		}
		else {
			partial_sort_copy(recList->begin(), recList->end(), m_pRecVec->begin(), m_pRecVec->end(), Descending());
		}
	}

	RecordingsList::RecordingsList(std::shared_ptr<RecordingsList> recList, time_t begin, time_t end, bool ascending) :
		m_pRecVec(new RecVecType())
	{
		if (end > begin) {
			return;
		}
		if (!m_pRecVec) {
			return;
		}
		remove_copy_if(recList->begin(), recList->end(), m_pRecVec->end(), NotInRange(begin, end));

		if (ascending) {
			sort(m_pRecVec->begin(), m_pRecVec->end(), Ascending());
		}
		else {
			sort(m_pRecVec->begin(), m_pRecVec->end(), Descending());
		}
	}

	RecordingsList::~RecordingsList()
	{
		if (m_pRecVec) {
			delete m_pRecVec, m_pRecVec = 0;
		}
	}


	RecordingsList::NotInRange::NotInRange(time_t begin, time_t end) :
		m_begin(begin),
		m_end(end)
	{
	}

	bool RecordingsList::NotInRange::operator()(RecordingsItemPtr const &x) const
	{
		return (x->StartTime() < m_begin) || (m_end >= x->StartTime());
	}


	/**
	 *  Implementation of class RecordingsList:
	 */
	RecordingsListPtr::RecordingsListPtr(RecordingsManagerPtr recManPtr, std::shared_ptr<RecordingsList> recList) :
		std::shared_ptr<RecordingsList>(recList),
		m_recManPtr(recManPtr)
	{
	}

	RecordingsListPtr::~RecordingsListPtr()
	{
	}


	/**
	 *  Implementation of class DirectoryList:
	 */
	DirectoryList::DirectoryList(RecordingsManagerPtr recManPtr) :
		m_pDirVec(new DirVecType())
	{
		if (!m_pDirVec) {
			return;
		}

		m_pDirVec->push_back(""); // always add root directory
		for (cNestedItem* item = Folders.First(); item; item = Folders.Next(item)) { // add folders.conf entries
			InjectFoldersConf(item);
		}
#if VDRVERSNUM >= 20301
		LOCK_RECORDINGS_READ;
		for (cRecording* recording = (cRecording *)Recordings->First(); recording; recording = (cRecording*)Recordings->Next(recording)) {
#else
		for (cRecording* recording = Recordings.First(); recording; recording = Recordings.Next(recording)) {
#endif
			std::string name = recording->Name();
			size_t found = name.find_last_of("~");

			if (found != std::string::npos) {
				m_pDirVec->push_back(StringReplace(name.substr(0, found), "~", "/"));
			}
		}
		m_pDirVec->sort();
		m_pDirVec->unique();
	}

	DirectoryList::~DirectoryList()
	{
		if (m_pDirVec) {
			delete m_pDirVec, m_pDirVec = 0;
		}
	}

	void DirectoryList::InjectFoldersConf(cNestedItem * folder, std::string parent)
	{
		if (!folder) {
			return;
		}

		std::string dir = std::string((parent.size() == 0) ? "" : parent + "/") + folder->Text();
		m_pDirVec->push_back(StringReplace(dir, "_", " "));

		if (!folder->SubItems()) {
			return;
		}

		for(cNestedItem* item = folder->SubItems()->First(); item; item = folder->SubItems()->Next(item)) {
			InjectFoldersConf(item, dir);
		}
	}

	/**
	 *  Implementation of class DirectoryListPtr:
	 */
	DirectoryListPtr::DirectoryListPtr(RecordingsManagerPtr recManPtr, std::shared_ptr<DirectoryList> recDirs) :
		std::shared_ptr<DirectoryList>(recDirs),
		m_recManPtr(recManPtr)
	{
	}

	DirectoryListPtr::~DirectoryListPtr()
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

bool checkNew(RecordingsTreePtr recordingsTree, std::vector<std::string> p) {
        bool newR = false;
        RecordingsMap::iterator iter;
        for (iter = recordingsTree->begin(p); iter != recordingsTree->end(p); iter++) {
                RecordingsItemPtr recItem = iter->second;
                if(!recItem->IsDir())
                        newR |= recItem->Recording()->GetResume() <= 0;
                else {
                        std::vector<std::string> pp(p);
                        pp.push_back(recItem->Name());
                        newR |= checkNew(recordingsTree, pp);
                }
        }
        return newR;
}

void addAllRecordings(std::list<RecordingsItemPtr> &RecItems, RecordingsTreePtr &RecordingsTree, std::vector<std::string> &P){
  for (RecordingsMap::iterator iter = RecordingsTree->begin(P); iter != RecordingsTree->end(P);  ++iter) {
        RecordingsItemPtr recItem = iter->second;
        if (!recItem->IsDir()) {
                RecItems.push_back(recItem);
        } else {
                std::vector<std::string> pp(P);
                pp.push_back(recItem->Name());
                addAllRecordings(RecItems, RecordingsTree, pp);
        }
  }
}

void addAllDuplicateRecordings(std::list<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree){
  std::vector<std::string> path;
  std::list<RecordingsItemPtr> recItems;
  std::list<RecordingsItemPtr>::iterator currentRecItem, recIterUpName, recIterLowName, recIterUpShortText, recIterLowShortText;
  int numberOfRecordingsWithThisName;
  bool isSeries;

  addAllRecordings(recItems, RecordingsTree, path);
  recItems.sort(RecordingsItemPtrCompare::ByAscendingNameShortText);

  for (currentRecItem = recItems.begin(); currentRecItem != recItems.end(); ){
    recIterLowName = currentRecItem;
    recIterUpName  = std::upper_bound (currentRecItem , recItems.end(), *currentRecItem, RecordingsItemPtrCompare::ByAscendingName);

    currentRecItem++;
    if (recIterLowName == recIterUpName ) continue; // there is no recording with this name (internal error)
    if (currentRecItem == recIterUpName ) continue; // there is only one recording with this name
    for( numberOfRecordingsWithThisName = 1; currentRecItem != recIterUpName; currentRecItem++, numberOfRecordingsWithThisName++);
    if (numberOfRecordingsWithThisName > 5) isSeries = true; else isSeries = false;
    if (isSeries) {
      for (currentRecItem = recIterLowName; currentRecItem != recIterUpName;){
        recIterLowShortText = currentRecItem;
        recIterUpShortText  = std::upper_bound (currentRecItem , recIterUpName, *currentRecItem, RecordingsItemPtrCompare::ByAscendingNameShortText);
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
