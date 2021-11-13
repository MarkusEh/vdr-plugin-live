#ifndef VDR_LIVE_RECORDINGS_H
#define VDR_LIVE_RECORDINGS_H

#include "stdext.h"
#include "setup.h"
#include "tools.h"

// STL headers need to be before VDR tools.h (included by <vdr/recording.h>)
#include <map>
#include <string>
#include <vector>
#include <list>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any vdr include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/recording.h>

namespace vdrlive {

	// Forward declations from epg_events.h
	class EpgInfo;
	typedef std::shared_ptr<EpgInfo> EpgInfoPtr;

	/**
	 *  Some forward declarations
	 */
	class RecordingsManager;
	class RecordingsTree;
	class RecordingsTreePtr;
	class RecordingsList;
	class RecordingsListPtr;
	class DirectoryList;
	class DirectoryListPtr;
	class RecordingsItem;

	typedef std::shared_ptr<RecordingsManager> RecordingsManagerPtr;
	typedef std::shared_ptr<RecordingsItem> RecordingsItemPtr;
	typedef std::weak_ptr<RecordingsItem> RecordingsItemWeakPtr;
	typedef std::multimap<std::string, RecordingsItemPtr> RecordingsMap;


	/**
	 *  Class for managing recordings inside the live plugin. It
	 *  provides some convenience methods and provides automatic
	 *  locking during requests on the recordings or during the
	 *  traversion of the recordings tree or lists, which can only be
	 *  obtained through methods of the RecordingsManager.
	 */
	class RecordingsManager
	{
		friend RecordingsManagerPtr LiveRecordingsManager();

		public:
			/**
			 *  Returns a shared pointer to a fully populated
			 *  recordings tree.
			 */
			RecordingsTreePtr GetRecordingsTree() const;

			/**
			 *  Return a shared pointer to a populated recordings
			 *  list. The list is optionally sorted ascending or
			 *  descending by date and may be constrained by a date
			 *  range.
			 */
			RecordingsListPtr GetRecordingsList(bool ascending = true) const;
			RecordingsListPtr GetRecordingsList(time_t begin, time_t end, bool ascending = true) const;

			/**
			 *  Returns a shared pointer to a fully populated
			 *  directory list.
			 */
			DirectoryListPtr GetDirectoryList() const;

			/**
			 *	generates a Md5 hash from a cRecording entry. It can be used
			 *  to reidentify a recording.
			 */
			std::string Md5Hash(cRecording const * recording) const;

			/**
			 *  fetches a cRecording from VDR's Recordings collection. Returns
			 *  NULL if recording was not found
			 */
			cRecording const* GetByMd5Hash(std::string const & hash) const;

			/**
			 *  Move a recording with the given hash according to
			 *  VDRs recording mechanisms.
			 */
			bool MoveRecording(cRecording const * recording, std::string const & name, bool copy = false) const;

			/**
			 *  Delete recording resume with the given hash according to
			 *  VDRs recording mechanisms.
			 */
			void DeleteResume(cRecording const * recording) const;

			/**
			 *  Delete recording marks with the given hash according to
			 *  VDRs recording mechanisms.
			 */
			void DeleteMarks(cRecording const * recording) const;

			/**
			 *  Delete a recording with the given hash according to
			 *  VDRs recording deletion mechanisms.
			 */
			void DeleteRecording(cRecording const * recording) const;

			/**
			 *	Determine wether the recording has been archived on
			 *	removable media (e.g. DVD-ROM)
			 */
			static int GetArchiveType(cRecording const * recording);

			/**
			 *	Provide an identification of the removable media
			 *	(e.g. DVD-ROM Number or Name) where the recording has
			 *	been archived.
			 */
			static std::string const GetArchiveId(cRecording const * recording, int archiveType);

			static std::string const GetArchiveDescr(cRecording const * recording);

		private:
			RecordingsManager();

#if VDRVERSNUM >= 20301
			static bool StateChanged();
#endif
			static RecordingsManagerPtr EnsureValidData();

			static std::weak_ptr<RecordingsManager> m_recMan;
			static std::shared_ptr<RecordingsTree> m_recTree;
			static std::shared_ptr<RecordingsList> m_recList;
			static std::shared_ptr<DirectoryList> m_recDirs;
#if VDRVERSNUM >= 20301
			static cStateKey m_recordingsStateKey;
#else
			static int m_recordingsState;
#endif

			cThreadLock m_recordingsLock;
	};

        class ShortTextDescription
        {
              public:
                   ShortTextDescription(const char * ShortText, const char * Description);
                   wint_t getNextNonPunctChar();
              private:
                   const char * m_short_text;
                   const char * m_description;
        };

	/**
	 * Class containing possible recordings compare functions
	 */
	class RecordingsItemPtrCompare
	{
		public:
			static bool ByAscendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByDescendingDate(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByAscendingName(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByAscendingNameShortText(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByAscendingNameDesc(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByAscendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByDescendingNameDescSort(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static bool ByDescendingRecordingErrors(const RecordingsItemPtr & first, const RecordingsItemPtr & second);
			static std::string getNameForSort(const std::string &Name);
                        static int compareLC(int &numEqualChars, const char *first, const char *second); // as std::compare, but compare lower case
                        static int Compare(int &numEqualChars, const RecordingsItemPtr &first, const RecordingsItemPtr &second);
                        static int Compare2(int &numEqualChars, const RecordingsItemPtr &first, const RecordingsItemPtr &second);
                        static int FindBestMatch(RecordingsItemPtr &BestMatch, const std::list<RecordingsItemPtr>::iterator & First, const std::list<RecordingsItemPtr>::iterator & Last, const RecordingsItemPtr & EPG_Entry);

	};

	/**
	 *  Base class for entries in recordings tree and recordings list.
	 *  All opeations possible on recordings are performed against
	 *  pointers to instances of recordings items. The C++ polymorphy
	 *  delegates them to the 'right' class.
	 */
	class RecordingsItem
	{
		friend class RecordingsTree;

		protected:
			RecordingsItem(const std::string& name, RecordingsItemPtr parent);

		public:
			virtual ~RecordingsItem();

			virtual time_t StartTime() const = 0;
			virtual bool IsDir() const = 0;
			virtual long Duration() const = 0;
			virtual const std::string& Name() const { return m_name; }
			virtual const std::string& NameForSort() const { return m_name_for_sort; }
			virtual const std::string& NameForSearch() const { return m_name_for_search; }
                        virtual const char * ShortText() const { return RecInfo()? RecInfo()->ShortText():0; }
                        virtual const char * Description() const { return RecInfo()? RecInfo()->Description():0; }
			virtual const std::string Id() const = 0;

			virtual const cRecording* Recording() const { return 0; }
			virtual const cRecordingInfo* RecInfo() const { return 0; }

			RecordingsMap::const_iterator begin() const { return m_entries.begin(); }
			RecordingsMap::const_iterator end() const { return m_entries.end(); }
			int Level() { return m_level; }

           // To display the recuring on the UI
                        virtual const int IsArchived() const { return 0 ; }
                        virtual const std::string ArchiveDescr() const { return "" ; }
                        virtual const char *NewR() const { return "" ; }
                        virtual const int RecordingErrors() const { return -1; }
                        virtual const char *RecordingErrorsIcon() const { return ""; }
                        virtual void AppendRecordingErrorsStr(std::string &target) const { };
                        virtual const int SD_HD() { return 0; }
                        virtual const char *SD_HD_icon() { return ""; }
                        virtual void AppendasHtml(std::string &target, bool displayFolder, const std::string argList) { }

		private:
			std::string GetNameForSearch(std::string const & name);
			int m_level;
			const std::string m_name;
			const std::string m_name_for_sort;
			const std::string m_name_for_search;
			RecordingsMap m_entries;
			RecordingsItemWeakPtr m_parent;
	};


	/**
	 *  A recordings item that resembles a directory with other
	 *  subdirectories and/or real recordings.
	 */
	class RecordingsItemDir : public RecordingsItem
	{
		public:
			RecordingsItemDir(const std::string& name, int level, RecordingsItemPtr parent);

			virtual ~RecordingsItemDir();

			virtual time_t StartTime() const { return 0; }
			virtual long Duration() const { return 0; }
			virtual bool IsDir() const { return true; }
			virtual std::string const Id() const { return ""; }

		private:
			int m_level;
	};


	/**
	 *  A recordings item that represents a real recording. This is
	 *  the leaf item in the recordings tree or one of the items in
	 *  the recordings list.
	 */
	class RecordingsItemRec : public RecordingsItem
	{
		public:
			RecordingsItemRec(const std::string& id, const std::string& name, const cRecording* recording, RecordingsItemPtr parent);

			virtual ~RecordingsItemRec();

			virtual time_t StartTime() const { return m_recording->Start(); }
			virtual long Duration() const { return m_duration; }
			virtual bool IsDir() const { return false; }
			virtual const std::string Id() const { return m_id; }

			virtual const cRecording* Recording() const { return m_recording; }
			virtual const cRecordingInfo* RecInfo() const { return m_recording->Info(); }

           // To display the recuring on the UI
                        virtual const int IsArchived() const { return m_isArchived ; }
                        virtual const std::string ArchiveDescr() const { return RecordingsManager::GetArchiveDescr(m_recording) ; }
                        virtual const char *NewR() const { return LiveSetup().GetMarkNewRec() && (Recording()->GetResume() <= 0) ? "_new" : "" ; }
#if VDRVERSNUM >= 20505
                        virtual const int RecordingErrors() const { return RecInfo()->Errors(); }
#else
                        virtual const int RecordingErrors() const { return -1; }
#endif
                        virtual const char *RecordingErrorsIcon() const;
                        void AppendRecordingErrorsStr(std::string &target) const;

                        virtual const int SD_HD();
                        virtual const char *SD_HD_icon() { return SD_HD() == 0 ? "sd.png": "hd.png"; }
                        virtual void AppendasHtml(std::string &target, bool displayFolder, const std::string argList);
                        void AppendHint(std::string &target) const;
                        void AppendIMDb(std::string &target) const;
                        void AppendRecordingAction(std::string &target, const char *A, const char *Img, const char *Title, const std::string argList);
                     
		private:
			const cRecording *m_recording;
			const std::string m_id;
                        const int m_isArchived;
                        const long m_duration; // duration in minutes
                        int m_video_SD_HD = -1;  // 0 is SD, 1 is HD
	};

	/**
	 * Class containing recordings to compare or "dummy" recordings, i.e. data from EPG which can be compared with a recording
	 */
	class RecordingsItemDummy: public RecordingsItem
	{
		public:
			RecordingsItemDummy(const std::string &Name, const std::string &ShortText, const std::string &Description, long Duration);

			~RecordingsItemDummy() { };

			const char * ShortText() const { return m_short_text; }
			const char * Description() const { return m_description; }
                        virtual time_t StartTime() const { return 0; }
                        virtual long Duration() const { return m_duration; } // duration in minutes
                        virtual bool IsDir() const { return false; }
                        virtual std::string const Id() const { return ""; }


		private:
                        const char * m_short_text;
                        const char * m_description;
                        const long m_duration;
        };

	/**
	 *  The recordings tree contains all recordings in a file system
	 *  tree like fashion.
	 */
	class RecordingsTree
	{
		friend class RecordingsManager;

		private:
			RecordingsTree(RecordingsManagerPtr recManPtr);

		public:
			virtual ~RecordingsTree();

			RecordingsItemPtr const & Root() const { return m_root; }

			RecordingsMap::iterator begin(const std::vector<std::string>& path);
			RecordingsMap::iterator end(const std::vector<std::string>&path);

			int MaxLevel() const { return m_maxLevel; }

		private:
			int m_maxLevel;
			RecordingsItemPtr m_root;

			RecordingsMap::iterator findDir(RecordingsItemPtr& dir, const std::string& dirname);
	};


	/**
	 *  A smart pointer to a recordings tree. As long as an instance of this
	 *  exists the recordings are locked in the vdr.
	 */
	class RecordingsTreePtr : public std::shared_ptr<RecordingsTree>
	{
		friend class RecordingsManager;

		private:
			RecordingsTreePtr(RecordingsManagerPtr recManPtr, std::shared_ptr<RecordingsTree> recTree);

		public:
			RecordingsTreePtr();
			virtual ~RecordingsTreePtr();

		private:
			RecordingsManagerPtr m_recManPtr;
	};


	/**
	 *  The recordings list contains all real recordings in a list
	 *  sorted by a given sorting predicate function. The directory
	 *  entries are not part of this list. The path towards the root
	 *  can be obtained via the 'parent' members of the recordings
	 *  items.
	 */
	class RecordingsList
	{
		friend class RecordingsManager;

		private:
			RecordingsList(RecordingsTreePtr recTree);
			RecordingsList(std::shared_ptr<RecordingsList> recList, bool ascending);
			RecordingsList(std::shared_ptr<RecordingsList> recList, time_t begin, time_t end, bool ascending);

		public:
			typedef std::vector<RecordingsItemPtr> RecVecType;

			virtual ~RecordingsList();

			RecVecType::const_iterator begin() const { return m_pRecVec->begin(); }
			RecVecType::const_iterator end() const { return m_pRecVec->end(); }

			RecVecType::size_type size() const { return m_pRecVec->size(); }

		private:
			class Ascending
			{
				public:
					bool operator()(RecordingsItemPtr const &x, RecordingsItemPtr const &y) const { return x->StartTime() < y->StartTime(); }
			};

			class Descending
			{
				public:
					bool operator()(RecordingsItemPtr const &x, RecordingsItemPtr const &y) const { return y->StartTime() < x->StartTime(); }
			};

			class NotInRange
			{
				public:
					NotInRange(time_t begin, time_t end);

					bool operator()(RecordingsItemPtr const &x) const;

				private:
					time_t m_begin;
					time_t m_end;
			};

		private:
			RecVecType *m_pRecVec;
	};


	/**
	 *  A smart pointer to a recordings list. As long as an instance of this
	 *  exists the recordings are locked in the vdr.
	 */
	class RecordingsListPtr : public std::shared_ptr<RecordingsList>
	{
		friend class RecordingsManager;

		private:
			RecordingsListPtr(RecordingsManagerPtr recManPtr, std::shared_ptr<RecordingsList> recList);

		public:
			virtual ~RecordingsListPtr();

		private:
			RecordingsManagerPtr m_recManPtr;
	};


	/**
	 *  The recording directory list contains all real directory entries.
	 */
	class DirectoryList
	{
		friend class RecordingsManager;

		private:
			DirectoryList(RecordingsManagerPtr recManPtr);
			void InjectFoldersConf(cNestedItem * folder, std::string parent = "");

		public:
			typedef std::list<std::string> DirVecType;

			virtual ~DirectoryList();

			DirVecType::const_iterator begin() const { return m_pDirVec->begin(); }
			DirVecType::const_iterator end() const { return m_pDirVec->end(); }
			DirVecType::size_type size() const { return m_pDirVec->size(); }

		private:
			DirVecType *m_pDirVec;
	};


	/**
	 *  A smart pointer to a directory list. As long as an instance of this
	 *  exists the recordings are locked in the vdr.
	 */
	class DirectoryListPtr : public std::shared_ptr<DirectoryList>
	{
		friend class RecordingsManager;

		private:
			DirectoryListPtr(RecordingsManagerPtr recManPtr, std::shared_ptr<DirectoryList> recDirs);

		public:
			virtual ~DirectoryListPtr();

		private:
			RecordingsManagerPtr m_recManPtr;
	};


	/**
	 *	return singleton instance of RecordingsManager as a shared Pointer.
	 *  This ensures that after last use of the RecordingsManager it is
	 *  deleted. After deletion of the original RecordingsManager a repeated
	 *  call to this function creates a new RecordingsManager which is again
	 *	kept alive as long references to it exist.
	 */
	RecordingsManagerPtr LiveRecordingsManager();

bool checkNew(RecordingsTreePtr recordingsTree, std::vector<std::string> p);

/**
* Create a (flat) list of all recordings.
* sample code to achieve this:
* std::vector<std::string> path;
* std::list<RecordingsItemPtr> recItems;
* RecordingsTreePtr recordingsTree(LiveRecordingsManager()->GetRecordingsTree());

* addAllRecordings(recItems, recordingsTree, path);
*/
void addAllRecordings(std::list<RecordingsItemPtr> &RecItems, RecordingsTreePtr &RecordingsTree, std::vector<std::string> &P);

void addAllDuplicateRecordings(std::list<RecordingsItemPtr> &DuplicateRecItems, RecordingsTreePtr &RecordingsTree);

} // namespace vdrlive

#endif // VDR_LIVE_RECORDINGS_H
