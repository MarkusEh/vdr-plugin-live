#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

#include <ctime>
#include <map>
#include <boost/shared_ptr.hpp>
#include <vdr/recording.h>

namespace vdrlive {

	using namespace boost;
	using namespace std;

	class RecordingsTree
	{
		public:

			class RecordingsItem;

			typedef shared_ptr< RecordingsItem > RecordingsItemPtr;
			typedef map< string, RecordingsItemPtr > Map;

			class RecordingsItem
			{
				friend class RecordingsTree;

				public:
					virtual ~RecordingsItem();
					virtual time_t StartTime() const = 0;
					virtual bool IsDir() const = 0;
					virtual const char* Name() const = 0;

				protected:
					RecordingsItem();

				private:
					Map m_entries;
			};

			class RecordingsItemDir : public RecordingsItem
			{
				public:
					RecordingsItemDir();
					virtual ~RecordingsItemDir();
					RecordingsItemDir(const string& name, int level);

					virtual time_t StartTime() const { return 0; }
					virtual bool IsDir() const { return true; }
					virtual const char* Name() const { return m_name.c_str(); }

				private:
					string m_name;
					int m_level;
			};

			class RecordingsItemRec : public RecordingsItem
			{
				public:
					RecordingsItemRec(cRecording* recording);
					virtual ~RecordingsItemRec();

					virtual time_t StartTime() const;
					virtual bool IsDir() const { return false; }
					virtual const char* Name() const { return m_recording->Name(); }

				private:
					cRecording *m_recording;
			};

			RecordingsTree();
			virtual ~RecordingsTree();

			Map::iterator begin() { return m_root->m_entries.begin(); }
			Map::iterator end() { return m_root->m_entries.end(); }

			int MaxLevel() const { return m_maxLevel; }

		private:
			int m_maxLevel;
			RecordingsItemPtr m_root;
			cThreadLock m_recordingsLock;
	};

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
