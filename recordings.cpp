#include <unistd.h>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include "tools.h"
#include "epg_events.h"
#include "recordings.h"
#include "stdext.h"


using namespace std::tr1;
using namespace std;

namespace vdrlive {

	RecordingsManager::RecordingsManager() :
		m_recordingsLock(&Recordings)
	{
	}

	string RecordingsManager::Md5Hash(const cRecording* recording) const
	{
		return "recording_" + MD5Hash(recording->FileName());
/*		unsigned char md5[MD5_DIGEST_LENGTH];
		const char* fileName = recording->FileName();
		MD5(reinterpret_cast<const unsigned char*>(fileName), strlen(fileName), md5);

		ostringstream hashStr;
		hashStr << "MD5" << hex;
		for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++)
			hashStr << (0 + md5[i]);
		return hashStr.str();
*/
	}

	const cRecording* RecordingsManager::GetByMd5Hash(const string& hash) const
	{
		if (!hash.empty()) {
			for (cRecording* rec = Recordings.First(); rec != 0; rec = Recordings.Next(rec)) {
				if (hash == Md5Hash(rec))
					return rec;
			}
		}
		return 0;
	}

	bool RecordingsManager::IsArchived(const cRecording* recording)
	{
		string filename = recording->FileName();

		string vdrFile = filename + "/001.vdr";
		if (0 == access(vdrFile.c_str(), R_OK))
			return false;

		filename += "/dvd.vdr";
		return (0 == access(filename.c_str(), R_OK));
	}

	const std::string RecordingsManager::GetArchiveId(const cRecording* recording)
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

	const string RecordingsManager::GetArchiveDescr(const cRecording* recording)
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


	RecordingsTree::RecordingsTree(RecordingsManagerPtr recMan) :
		m_maxLevel(0),
		m_root(new RecordingsItemDir()),
		m_recManPtr(recMan)
	{
		// esyslog("DH: ****** RecordingsTree::RecordingsTree() ********");
		for ( cRecording* recording = Recordings.First(); recording != 0; recording = Recordings.Next( recording ) ) {
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
					Map::iterator i = findDir(dir, dirName);
					if (i == dir->m_entries.end()) {
						RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, level));
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
					RecordingsItemPtr recPtr (new RecordingsItemRec(m_recManPtr->Md5Hash(recording), recName, recording));
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

	RecordingsTree::Map::iterator RecordingsTree::begin(const vector< string >& path)
	{
		if (path.empty()) {
			return m_root->m_entries.begin();
		}

		RecordingsItemPtr recItem = m_root;
		for (vector< string >::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			pair< Map::iterator, Map::iterator> range = recItem->m_entries.equal_range(*i);
			for (Map::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.begin();
	}

	RecordingsTree::Map::iterator RecordingsTree::end(const vector< string >&path)
	{
		if (path.empty()) {
			return m_root->m_entries.end();
		}

		RecordingsItemPtr recItem = m_root;
		for (vector< string >::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			pair< Map::iterator, Map::iterator> range = recItem->m_entries.equal_range(*i);
			for (Map::iterator iter = range.first; iter != range.second; ++iter) {
				if (iter->second->IsDir()) {
					recItem = iter->second;
					break;
				}
			}
		}
		return recItem->m_entries.end();
	}

	RecordingsTree::Map::iterator RecordingsTree::findDir(RecordingsItemPtr& dir, const string& dirName)
	{
		pair< Map::iterator, Map::iterator > range = dir->m_entries.equal_range(dirName);
		for (Map::iterator i = range.first; i != range.second; ++i) {
			if (i->second->IsDir()) {
				return i;
			}
		}
		return dir->m_entries.end();
	}

	RecordingsTree::RecordingsItem::RecordingsItem(const string& name) :
		m_name(name),
		m_entries()
	{
	}

	RecordingsTree::RecordingsItem::~RecordingsItem()
	{
	}

	RecordingsTree::RecordingsItemDir::RecordingsItemDir() :
		RecordingsItem(""),
		m_level(0)
	{
	}

	RecordingsTree::RecordingsItemDir::~RecordingsItemDir()
	{
	}

	RecordingsTree::RecordingsItemDir::RecordingsItemDir(const string& name, int level) :
		RecordingsItem(name),
		m_level(level)
	{
	}

	RecordingsTree::RecordingsItemRec::RecordingsItemRec(const string& id, const string& name, const cRecording* recording) :
		RecordingsItem(name),
		m_recording(recording),
		m_id(id)
	{
	}

	RecordingsTree::RecordingsItemRec::~RecordingsItemRec()
	{
	}

	time_t RecordingsTree::RecordingsItemRec::StartTime() const
	{
		return m_recording->start;
	}



	RecordingsManagerPtr LiveRecordingsManager()
	{
		static weak_ptr<RecordingsManager> livingRecMan;

		RecordingsManagerPtr r = livingRecMan.lock();
		if (r) {
			return r;
		}
		else {
			RecordingsManagerPtr n(new RecordingsManager);
			livingRecMan = n;
			return n;
		}
	}

} // namespace vdrlive
