#include <unistd.h>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <stack>
#include <algorithm>

#include "stdext.h"
#include "tools.h"

#include "epg_events.h"
#include "recman.h"

#define INDEXFILESUFFIX   "/index.vdr"

using namespace std::tr1;
using namespace std;

namespace vdrlive {

	/**
	 *  Implementation of class RecordingsManager:
	 */
	weak_ptr< RecordingsManager > RecordingsManager::m_recMan;
	shared_ptr< RecordingsTree > RecordingsManager::m_recTree;
	shared_ptr< RecordingsList > RecordingsManager::m_recList;
	int RecordingsManager::m_recordingsState = 0;

	// The RecordingsManager holds a VDR lock on the
	// Recordings. Additionally the singleton instance of
	// RecordingsManager is held in a weak pointer. If it is not in
	// use any longer, it will be freed automaticaly, which leads to a
	// release of the VDR recordings lock. Upon requesting access to
	// the RecordingsManager via LiveRecordingsManger function, first
	// the weak ptr is locked (obtaining a shared_ptr from an possible
	// existing instance) and if not successfull a new instance is
	// created, which again locks the VDR Recordings.
	//
	// RecordingsManager provides factory methods to obtain other
	// recordings data structures. The data structures returned keep if
	// needed the instance of RecordingsManager alive until destructed
	// themselfs. This way the use of LIVE::recordings is straight
	// forward and does hide the locking needs from the user.

	RecordingsManager::RecordingsManager() :
		m_recordingsLock(&Recordings)
	{
	}

	RecordingsTreePtr RecordingsManager::GetRecordingsTree() const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsTreePtr(recMan, shared_ptr< RecordingsTree >());
		}
		return RecordingsTreePtr(recMan, m_recTree);
	}

	RecordingsListPtr RecordingsManager::GetRecordingsList(bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, shared_ptr< RecordingsList >());
		}
		return RecordingsListPtr(recMan, shared_ptr< RecordingsList >(new RecordingsList(m_recList, ascending)));
	}

	RecordingsListPtr RecordingsManager::GetRecordingsList(time_t begin, time_t end, bool ascending) const
	{
		RecordingsManagerPtr recMan = EnsureValidData();
		if (! recMan) {
			return RecordingsListPtr(recMan, shared_ptr< RecordingsList >());
		}
		return RecordingsListPtr(recMan, shared_ptr< RecordingsList >(new RecordingsList(m_recList, ascending)));
	}

	string RecordingsManager::Md5Hash(cRecording const * recording) const
	{
		return "recording_" + MD5Hash(recording->FileName());
	}

	cRecording const * RecordingsManager::GetByMd5Hash(string const & hash) const
	{
		if (!hash.empty()) {
			for (cRecording* rec = Recordings.First(); rec != 0; rec = Recordings.Next(rec)) {
				if (hash == Md5Hash(rec))
					return rec;
			}
		}
		return 0;
	}

	void RecordingsManager::DeleteRecording(cRecording const * recording) const
	{
		if (!recording)
			return;

		string name(recording->FileName());
		const_cast<cRecording *>(recording)->Delete();
		Recordings.DelByName(name.c_str());
	}

	bool RecordingsManager::IsArchived(cRecording const * recording)
	{
		string filename = recording->FileName();

		string vdrFile = filename + "/001.vdr";
		if (0 == access(vdrFile.c_str(), R_OK))
			return false;

		filename += "/dvd.vdr";
		return (0 == access(filename.c_str(), R_OK));
	}

	string const RecordingsManager::GetArchiveId(cRecording const * recording)
	{
		string filename = recording->FileName();

		filename += "/dvd.vdr";
		ifstream dvd(filename.c_str());

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
		return "";
	}

	string const RecordingsManager::GetArchiveDescr(cRecording const * recording)
	{
		string archived;
		if (IsArchived(recording)) {
			archived += " [";
			archived += tr("On archive DVD No.");
			archived += ": ";
			archived += GetArchiveId(recording);
			archived += "]";
		}
		return archived;
	}

	RecordingsManagerPtr RecordingsManager::EnsureValidData()
	{
		// Get singleton instance of RecordingsManager.  'this' is not
		// an instance of shared_ptr of the singleton
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
		bool stateChanged = Recordings.StateChanged(m_recordingsState);
		if (stateChanged || (!m_recTree) || (!m_recList)) {
			if (stateChanged) {
				m_recTree.reset();
				m_recList.reset();
			}
			if (stateChanged || !m_recTree) {
				m_recTree = shared_ptr< RecordingsTree >(new RecordingsTree(recMan));
			}
			if (!m_recTree) {
				esyslog("[LIVE]: creation of recordings tree failed!");
				return RecordingsManagerPtr();
			}
			if (stateChanged || !m_recList) {
				m_recList = shared_ptr< RecordingsList >(new RecordingsList(RecordingsTreePtr(recMan, m_recTree)));
			}
			if (!m_recList) {
				esyslog("[LIVE]: creation of recordings list failed!");
				return RecordingsManagerPtr();
			}
		}
		return recMan;
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
		return m_recording->start;
	}

	long RecordingsItemRec::Duration() const
	{
		long RecLength = 0;
		if (!m_recording->FileName()) return 0;
		cString filename = cString::sprintf("%s%s", m_recording->FileName(), INDEXFILESUFFIX);
		if (*filename) {
			if (access(filename, R_OK) == 0) {
				struct stat buf;
				if (stat(filename, &buf) == 0) {
					struct tIndex { int offset; uchar type; uchar number; short reserved; };
					int delta = buf.st_size % sizeof(tIndex);
					if (delta) {
						delta = sizeof(tIndex) - delta;
						esyslog("ERROR: invalid file size (%ld) in '%s'", buf.st_size, *filename);
					}
					RecLength = (buf.st_size + delta) / sizeof(tIndex) / SecondsToFrames(60);
				}
			}
		}
		return RecLength;
	}

	/**
	 *  Implementation of class RecordingsTree:
	 */
	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir("", 0, RecordingsItemPtr()))
	{
		// esyslog("DH: ****** RecordingsTree::RecordingsTree() ********");
		for (cRecording* recording = Recordings.First(); recording != 0; recording = Recordings.Next(recording)) {
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
						if (i != dir->m_entries.end()) {
							// esyslog("DH: added dir: '%s'", dirName.c_str());
						}
						else {
							// esyslog("DH: panic: didn't found inserted dir: '%s'", dirName.c_str());
						}
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
		shared_ptr<RecordingsTree>(),
		m_recManPtr()
	{
	}

	RecordingsTreePtr::RecordingsTreePtr(RecordingsManagerPtr recManPtr, std::tr1::shared_ptr< RecordingsTree > recTree) :
		shared_ptr<RecordingsTree>(recTree),
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

		stack< RecordingsItemPtr > treeStack;
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

	RecordingsList::RecordingsList(shared_ptr< RecordingsList > recList, bool ascending) :
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

	RecordingsList::RecordingsList(shared_ptr< RecordingsList > recList, time_t begin, time_t end, bool ascending) :
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
	RecordingsListPtr::RecordingsListPtr(RecordingsManagerPtr recManPtr, shared_ptr< RecordingsList > recList) :
		shared_ptr< RecordingsList >(recList),
		m_recManPtr(recManPtr)
	{
	}

	RecordingsListPtr::~RecordingsListPtr()
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
