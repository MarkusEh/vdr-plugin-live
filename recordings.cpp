#include <string>
#include <boost/shared_ptr.hpp>
#include "recordings.h"

namespace vdrlive {

	RecordingsTree::RecordingsTree() :
		m_maxLevel(0),
		m_root(new RecordingsItemDir()),
		m_recordingsLock(&Recordings)

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
					RecordingsItemPtr recPtr (new RecordingsItemRec(recName, recording));
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
			Map::iterator iter = recItem->m_entries.find(*i);
			recItem = iter->second;
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
			Map::iterator iter = recItem->m_entries.find(*i);
			recItem = iter->second;
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

	RecordingsTree::RecordingsItemRec::RecordingsItemRec(const string& name, cRecording* recording) :
		RecordingsItem(name),
		m_recording(recording)
	{
	}

	RecordingsTree::RecordingsItemRec::~RecordingsItemRec()
	{
	}

	time_t RecordingsTree::RecordingsItemRec::StartTime() const
	{
		return m_recording->start;
	}

} // namespace vdrlive
