#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

#include <ctime>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <vdr/recording.h>

namespace vdrlive {

	// Forward declations from epg_events.h
	class EpgEvent;
	typedef boost::shared_ptr<EpgEvent> EpgEventPtr;

	class RecordingsManager;
	typedef boost::shared_ptr<RecordingsManager> RecordingsManagerPtr;

	class RecordingsManager
	{
		friend RecordingsManagerPtr LiveRecordingsManager();

		public:
			/**
			 *	generates a Md5 hash from a cRecording entry. It can be used
			 *  to reidentify a recording.
			 */
			std::string Md5Hash(const cRecording* recording) const;

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

			typedef boost::shared_ptr< RecordingsItem > RecordingsItemPtr;
			typedef std::multimap< std::string, RecordingsItemPtr > Map;

			class RecordingsItem
			{
				friend class RecordingsTree;

				public:
					virtual ~RecordingsItem();

					virtual time_t StartTime() const = 0;
					virtual bool IsDir() const = 0;
					virtual bool IsArchived() const = 0;
					virtual const std::string& Name() const { return m_name; }
					virtual const std::string Id() const = 0;
					virtual const std::string ArchiveId() const = 0;

					virtual const cRecording* Recording() const { return 0; }
					virtual const cRecordingInfo* RecInfo() const { return 0; }

				protected:
					RecordingsItem(const std::string& name);

				private:
					std::string m_name;
					Map m_entries;
			};

			class RecordingsItemDir : public RecordingsItem
			{
				public:
					RecordingsItemDir();
					RecordingsItemDir(const std::string& name, int level);

					virtual ~RecordingsItemDir();

					virtual time_t StartTime() const { return 0; }
					virtual bool IsDir() const { return true; }
					virtual bool IsArchived() const { return false; }
					virtual const std::string Id() const { std::string e; return e; }
					virtual const std::string ArchiveId() const { std::string e; return e; }

				private:
					int m_level;
			};

			class RecordingsItemRec : public RecordingsItem
			{
				public:
					RecordingsItemRec(const std::string& id, const std::string& name, cRecording* recording);

					virtual ~RecordingsItemRec();

					virtual time_t StartTime() const;
					virtual bool IsDir() const { return false; }
					virtual bool IsArchived() const ;
					virtual const std::string Id() const { return m_id; }
					virtual const std::string ArchiveId() const;

					virtual const cRecording* Recording() const { return m_recording; }
					virtual const cRecordingInfo* RecInfo() const { return m_recording->Info(); }

				private:
					cRecording *m_recording;
					std::string m_id;
			};

			RecordingsTree(RecordingsManagerPtr recManPtr);

			virtual ~RecordingsTree();

			Map::iterator begin(const std::vector< std::string >& path);
			Map::iterator end(const std::vector< std::string >&path);

			int MaxLevel() const { return m_maxLevel; }

			static EpgEventPtr CreateEpgEvent(const RecordingsItemPtr recItem);

		private:
			int m_maxLevel;
			RecordingsItemPtr m_root;
			RecordingsManagerPtr m_recManPtr;

			Map::iterator findDir(RecordingsItemPtr& dir, const std::string& dirname);
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
