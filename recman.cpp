
#include "recman.h"

#include "tools.h"

// STL headers need to be before VDR tools.h (included by <vdr/videodir.h>)
#include <fstream>
#include <stack>
#include <algorithm>

#include <vdr/videodir.h>

#define INDEXFILESUFFIX   "/index.vdr"
#define LENGTHFILESUFFIX  "/length.vdr"

using std::string;
using std::vector;
using std::pair;

namespace vdrlive {

	/**
	 *  Implementation of class RecordingsManager:
	 */
	std::tr1::weak_ptr< RecordingsManager > RecordingsManager::m_recMan;
	std::tr1::shared_ptr< RecordingsTree > RecordingsManager::m_recTree;
	std::tr1::shared_ptr< RecordingsList > RecordingsManager::m_recList;
	std::tr1::shared_ptr< DirectoryList > RecordingsManager::m_recDirs;
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
	// the weak ptr is locked (obtaining a std::tr1::shared_ptr from an possible
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
			return RecordingsTreePtr(recMan, std::tr1::shared_ptr< RecordingsTree >());
		}
		return RecordingsTreePtr(recMan, m_recTree);
	}

	RecordingsListPtr RecordingsManager::GetRecordingsList(bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, std::tr1::shared_ptr< RecordingsList >());
		}
		return RecordingsListPtr(recMan, std::tr1::shared_ptr< RecordingsList >(new RecordingsList(m_recList, ascending)));
	}

	RecordingsListPtr RecordingsManager::GetRecordingsList(time_t begin, time_t end, bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, std::tr1::shared_ptr< RecordingsList >());
		}
		return RecordingsListPtr(recMan, std::tr1::shared_ptr< RecordingsList >(new RecordingsList(m_recList, ascending)));
	}

	DirectoryListPtr RecordingsManager::GetDirectoryList() const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (!recMan) {
			return DirectoryListPtr(recMan, std::tr1::shared_ptr< DirectoryList >());
		}
		return DirectoryListPtr(recMan, m_recDirs);
	}

	string RecordingsManager::Md5Hash(cRecording const * recording) const
	{
		return "recording_" + MD5Hash(recording->FileName());
	}

	cRecording const * RecordingsManager::GetByMd5Hash(string const & hash) const
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

	bool RecordingsManager::MoveRecording(cRecording const * recording, string const & name, bool copy) const
	{
		if (!recording)
			return false;

		string oldname = recording->FileName();
		size_t found = oldname.find_last_of("/");

		if (found == string::npos)
			return false;

#if APIVERSNUM > 20101
		string newname = string(cVideoDirectory::Name()) + "/" + name + oldname.substr(found);
#else
		string newname = string(VideoDirectory) + "/" + name + oldname.substr(found);
#endif

		if (!MoveDirectory(oldname.c_str(), newname.c_str(), copy)) {
			esyslog("[LIVE]: renaming failed from '%s' to '%s'", oldname.c_str(), newname.c_str());
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

		string name(recording->FileName());
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
		string filename = recording->FileName();

		string dvdFile = filename + "/dvd.vdr";
		if (0 == access(dvdFile.c_str(), R_OK)) {
			return 1;
		}
		string hddFile = filename + "/hdd.vdr";
		if (0 == access(hddFile.c_str(), R_OK)) {
			return 2;
		}
		return 0;
	}

	string const RecordingsManager::GetArchiveId(cRecording const * recording, int archiveType)
	{
		string filename = recording->FileName();

		if (archiveType==1) {
			string dvdFile = filename + "/dvd.vdr";
			std::ifstream dvd(dvdFile.c_str());

		if (dvd) {
			string archiveDisc;
			string videoDisc;
			dvd >> archiveDisc;
			if ("0000" == archiveDisc) {
				dvd >> videoDisc;
				return videoDisc;
			}
			return archiveDisc;
		}
		} else if(archiveType==2) {
			string hddFile = filename + "/hdd.vdr";
			std::ifstream hdd(hddFile.c_str());

			if (hdd) {
				string archiveDisc;
				hdd >> archiveDisc;
				return archiveDisc;
			}
		}
		return "";
	}

	string const RecordingsManager::GetArchiveDescr(cRecording const * recording)
	{
		int archiveType;
		string archived;
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
		// an instance of std::tr1::shared_ptr of the singleton
		// RecordingsManager, so we obtain it in the overall
		// recommended way.
		RecordingsManagerPtr recMan = LiveRecordingsManager();
		if (! recMan) {
			// theoretically this code is never reached ...
			esyslog("[LIVE]: lost RecordingsManager instance while using it!");
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
				m_recTree = std::tr1::shared_ptr< RecordingsTree >(new RecordingsTree(recMan));
			}
			if (!m_recTree) {
				esyslog("[LIVE]: creation of recordings tree failed!");
				return RecordingsManagerPtr();
			}
			if (stateChanged || !m_recList) {
				m_recList = std::tr1::shared_ptr< RecordingsList >(new RecordingsList(RecordingsTreePtr(recMan, m_recTree)));
			}
			if (!m_recList) {
				esyslog("[LIVE]: creation of recordings list failed!");
				return RecordingsManagerPtr();
			}
			if (stateChanged || !m_recDirs) {
				m_recDirs = std::tr1::shared_ptr< DirectoryList >(new DirectoryList(recMan));
			}
			if (!m_recDirs) {
				esyslog("[LIVE]: creation of directory list failed!");
				return RecordingsManagerPtr();
			}

		}
		return recMan;
	}


	/**
	 * Implemetation of class RecordingsItemPtrCompare
	 */
	bool RecordingsItemPtrCompare::ByAscendingDate(RecordingsItemPtr & first, RecordingsItemPtr & second)
	{
		return (first->StartTime() < second->StartTime());
	}

	bool RecordingsItemPtrCompare::ByDescendingDate(RecordingsItemPtr & first, RecordingsItemPtr & second)
	{
		return (first->StartTime() >= second->StartTime());
	}

	bool RecordingsItemPtrCompare::ByAscendingName(RecordingsItemPtr & first, RecordingsItemPtr & second)
	{
		unsigned int i = 0;
		while (i < first->Name().length() && i < second->Name().length()) {
			if (tolower((first->Name())[i]) < tolower((second->Name())[i]))
				return true;
			else if (tolower((first->Name())[i]) > tolower((second->Name())[i]))
				return false;
			++i;
		}
		if (first->Name().length() < second->Name().length())
			return true;
		return false;
	}

	bool RecordingsItemPtrCompare::ByDescendingName(RecordingsItemPtr & first, RecordingsItemPtr & second)
	{
		unsigned int i = 0;
		while (i < first->Name().length() && i < second->Name().length()) {
			if (tolower((second->Name())[i]) < tolower((first->Name())[i]))
				return true;
			else if (tolower((second->Name())[i]) > tolower((first->Name())[i]))
				return false;
			++i;
		}
		if (second->Name().length() < first->Name().length())
			return true;
		return false;
	}


	/**
	 *  Implementation of class RecordingsItem:
	 */
	RecordingsItem::RecordingsItem(string const & name, RecordingsItemPtr parent) :
		m_name(name),
		m_entries(),
		m_parent(parent)
	{
	}

	RecordingsItem::~RecordingsItem()
	{
	}


	/**
	 *  Implementation of class RecordingsItemDir:
	 */
	RecordingsItemDir::RecordingsItemDir(const string& name, int level, RecordingsItemPtr parent) :
		RecordingsItem(name, parent),
		m_level(level)
	{
		// dsyslog("REC: C: dir %s -> %s", name.c_str(), parent ? parent->Name().c_str() : "ROOT");
	}

	RecordingsItemDir::~RecordingsItemDir()
	{
		// dsyslog("REC: D: dir %s", Name().c_str());
	}


	/**
	 *  Implementation of class RecordingsItemRec:
	 */
	RecordingsItemRec::RecordingsItemRec(const string& id, const string& name, const cRecording* recording, RecordingsItemPtr parent) :
		RecordingsItem(name, parent),
		m_recording(recording),
		m_id(id)
	{
		// dsyslog("REC: C: rec %s -> %s", name.c_str(), parent->Name().c_str());
	}

	RecordingsItemRec::~RecordingsItemRec()
	{
		// dsyslog("REC: D: rec %s", Name().c_str());
	}

	time_t RecordingsItemRec::StartTime() const
	{
		return m_recording->Start();
	}

	long RecordingsItemRec::Duration() const
	{
		if (!m_recording->FileName()) return 0;
		return m_recording->LengthInSeconds() / 60;
	}

	/**
	 *  Implementation of class RecordingsTree:
	 */
	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir("", 0, RecordingsItemPtr()))
	{
		// esyslog("DH: ****** RecordingsTree::RecordingsTree() ********");
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
			string name(recording->Name());

			// esyslog("DH: recName = '%s'", recording->Name());
			int level = 0;
			size_t index = 0;
			size_t pos = 0;
			do {
				pos = name.find('~', index);
				if (pos != string::npos) {
					string dirName(name.substr(index, pos - index));
					index = pos + 1;
					RecordingsMap::iterator i = findDir(dir, dirName);
					if (i == dir->m_entries.end()) {
						RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, level, dir));
						dir->m_entries.insert(pair< string, RecordingsItemPtr > (dirName, recPtr));
						i = findDir(dir, dirName);
#if 0
						if (i != dir->m_entries.end()) {
							// esyslog("DH: added dir: '%s'", dirName.c_str());
						}
						else {
							// esyslog("DH: panic: didn't found inserted dir: '%s'", dirName.c_str());
						}
#endif
					}
					dir = i->second;
					// esyslog("DH: current dir: '%s'", dir->Name().c_str());
					level++;
				}
				else {
					string recName(name.substr(index, name.length() - index));
					RecordingsItemPtr recPtr (new RecordingsItemRec(recMan->Md5Hash(recording), recName, recording, dir));
					dir->m_entries.insert(pair< string, RecordingsItemPtr > (recName, recPtr));
					// esyslog("DH: added rec: '%s'", recName.c_str());
				}
			} while (pos != string::npos);
		}
		// esyslog("DH: ------ RecordingsTree::RecordingsTree() --------");
	}

	RecordingsTree::~RecordingsTree()
	{
		// esyslog("DH: ****** RecordingsTree::~RecordingsTree() ********");
	}

	RecordingsMap::iterator RecordingsTree::begin(const vector< string >& path)
	{
		if (path.empty()) {
			return m_root->m_entries.begin();
		}

		RecordingsItemPtr recItem = m_root;
		for (vector< string >::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			pair< RecordingsMap::iterator, RecordingsMap::iterator> range = recItem->m_entries.equal_range(*i);
			for (RecordingsMap::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.begin();
	}

	RecordingsMap::iterator RecordingsTree::end(const vector< string >&path)
	{
		if (path.empty()) {
			return m_root->m_entries.end();
		}

		RecordingsItemPtr recItem = m_root;
		for (vector< string >::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			pair< RecordingsMap::iterator, RecordingsMap::iterator> range = recItem->m_entries.equal_range(*i);
			for (RecordingsMap::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.end();
	}

	RecordingsMap::iterator RecordingsTree::findDir(RecordingsItemPtr& dir, const string& dirName)
	{
		pair< RecordingsMap::iterator, RecordingsMap::iterator > range = dir->m_entries.equal_range(dirName);
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
		std::tr1::shared_ptr<RecordingsTree>(),
		m_recManPtr()
	{
	}

	RecordingsTreePtr::RecordingsTreePtr(RecordingsManagerPtr recManPtr, std::tr1::shared_ptr< RecordingsTree > recTree) :
		std::tr1::shared_ptr<RecordingsTree>(recTree),
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

		std::stack< RecordingsItemPtr > treeStack;
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

	RecordingsList::RecordingsList(std::tr1::shared_ptr< RecordingsList > recList, bool ascending) :
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

	RecordingsList::RecordingsList(std::tr1::shared_ptr< RecordingsList > recList, time_t begin, time_t end, bool ascending) :
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
	RecordingsListPtr::RecordingsListPtr(RecordingsManagerPtr recManPtr, std::tr1::shared_ptr< RecordingsList > recList) :
		std::tr1::shared_ptr< RecordingsList >(recList),
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
			string name = recording->Name();
			size_t found = name.find_last_of("~");

			if (found != string::npos) {
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

	void DirectoryList::InjectFoldersConf(cNestedItem * folder, string parent)
	{
		if (!folder) {
			return;
		}

		string dir = string((parent.size() == 0) ? "" : parent + "/") + folder->Text();
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
	DirectoryListPtr::DirectoryListPtr(RecordingsManagerPtr recManPtr, std::tr1::shared_ptr< DirectoryList > recDirs) :
		std::tr1::shared_ptr< DirectoryList >(recDirs),
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

} // namespace vdrlive
