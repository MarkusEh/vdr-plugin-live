#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

#include <ctime>
#include <map>
#include <vector>
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
			typedef multimap< string, RecordingsItemPtr > Map;

			class RecordingsItem
			{
				friend class RecordingsTree;

				public:
					virtual ~RecordingsItem();

					virtual time_t StartTime() const = 0;
					virtual bool IsDir() const = 0;
					virtual const string& Name() const { return m_name; }

				protected:
					RecordingsItem(const string& name);

				private:
					string m_name;
					Map m_entries;
			};

			class RecordingsItemDir : public RecordingsItem
			{
				public:
					RecordingsItemDir();
					RecordingsItemDir(const string& name, int level);

					virtual ~RecordingsItemDir();

					virtual time_t StartTime() const { return 0; }
					virtual bool IsDir() const { return true; }

				private:
					int m_level;
			};

			class RecordingsItemRec : public RecordingsItem
			{
				public:
					RecordingsItemRec(const string& name, cRecording* recording);

					virtual ~RecordingsItemRec();

					virtual time_t StartTime() const;
					virtual bool IsDir() const { return false; }

				private:
					cRecording *m_recording;
			};

			RecordingsTree();

			virtual ~RecordingsTree();

			Map::iterator begin(const vector< string >& path);
			Map::iterator end(const vector< string >&path);

			int MaxLevel() const { return m_maxLevel; }

		private:
			int m_maxLevel;
			RecordingsItemPtr m_root;
			cThreadLock m_recordingsLock;
	};

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
