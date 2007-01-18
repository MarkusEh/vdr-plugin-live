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

	class RecordingsManager;
	typedef shared_ptr<RecordingsManager> RecordingsManagerPtr;

	class RecordingsManager
	{
		friend RecordingsManagerPtr LiveRecordingsManager();

		public:
			/**
			 *	generates a Md5 hash from a cRecording entry. It can be used
			 *  to reidentify a recording.
			 */
			string Md5Hash(const cRecording* recording) const;

			/**
			 *  fetches a cRecording from VDR's Recordings collection. Returns
			 *  NULL if recording was not found
			 */
			const cRecording* GetByMd5Hash(const std::string& hash) const;

		private:
			RecordingsManager();

			cThreadLock m_recordingsLock;
	};


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
					virtual const string Id() const = 0;

					virtual const cRecording* Recording() const { return 0; }
					virtual const cRecordingInfo* RecInfo() const { return 0; }

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
					virtual const string Id() const { string e; return e; }

				private:
					int m_level;
			};

			class RecordingsItemRec : public RecordingsItem
			{
				public:
					RecordingsItemRec(const string& id, const string& name, cRecording* recording);

					virtual ~RecordingsItemRec();

					virtual time_t StartTime() const;
					virtual bool IsDir() const { return false; }
					virtual const string Id() const { return m_id; }

					virtual const cRecording* Recording() const { return m_recording; }
					virtual const cRecordingInfo* RecInfo() const { return m_recording->Info(); }

				private:
					cRecording *m_recording;
					string m_id;
			};

			RecordingsTree(RecordingsManagerPtr recManPtr);

			virtual ~RecordingsTree();

			Map::iterator begin(const vector< string >& path);
			Map::iterator end(const vector< string >&path);

			int MaxLevel() const { return m_maxLevel; }

		private:
			int m_maxLevel;
			RecordingsItemPtr m_root;
			RecordingsManagerPtr m_recManPtr;

			Map::iterator findDir(RecordingsItemPtr& dir, const string& dirname);
	};

	/**
	 *	return singleton instance of RecordingsManager as a shared Pointer.
	 *  This ensures that after last use of the RecordingsManager it is
	 *  deleted. After deletion of the original RecordingsManager a repeated
	 *  call to this function creates a new RecordingsManager which is again
	 *	kept alive as long references to it exist.
	 */
	RecordingsManagerPtr LiveRecordingsManager();

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
