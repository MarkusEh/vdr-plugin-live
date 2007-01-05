#include <string>
#include <boost/shared_ptr.hpp>
#include "recordings.h"

namespace vdrlive {

	RecordingsTree::RecordingsTree() :
		m_maxLevel(0),
		m_root(new RecordingsItemDir()),
		m_recordingsLock(&Recordings)

	{
		for ( cRecording* recording = Recordings.First(); recording != 0; recording = Recordings.Next( recording ) ) {
			if (m_maxLevel < recording->HierarchyLevels()) {
				m_maxLevel = recording->HierarchyLevels();
			}

			RecordingsItemPtr dir = m_root;
			string name(recording->Name());
			int level = 0;
			size_t index = 0;
			size_t pos = 0;
			do {
				pos = name.find('~', index);
				if (pos != string::npos) {
					string dirName(name.substr(index, pos - index));
					index = pos + 1;
					Map::iterator i = dir->m_entries.find(dirName);
					if (i == dir->m_entries.end()) {
						RecordingsItemPtr recPtr (new RecordingsItemDir(dirName, level));
						dir->m_entries[dirName] = recPtr;
					}
					dir = dir->m_entries[dirName];
					level++;
				}
				else {
					RecordingsItemPtr recPtr (new RecordingsItemRec(recording));
					dir->m_entries[name] = recPtr;
				}
			} while (pos != string::npos);
		}
	}

	RecordingsTree::~RecordingsTree()
	{
	}

	RecordingsTree::RecordingsItem::RecordingsItem() :
		m_entries()
	{
	}

	RecordingsTree::RecordingsItem::~RecordingsItem()
	{
	}

	RecordingsTree::RecordingsItemDir::RecordingsItemDir() :
		m_name(),
		m_level(0)
	{
	}

	RecordingsTree::RecordingsItemDir::~RecordingsItemDir()
	{
	}

	RecordingsTree::RecordingsItemDir::RecordingsItemDir(const string& name, int level) :
		m_name(name),
		m_level(level)
	{
	}

	RecordingsTree::RecordingsItemRec::RecordingsItemRec(cRecording* recording) :
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
