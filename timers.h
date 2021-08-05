#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

// STL headers need to be before VDR tools.h (included by <vdr/timers.h>)
#include <list>
#include <string>

#if TNTVERSION >= 30000
        #include <cxxtools/log.h>  // must be loaded before any vdr include because of duplicate macros (LOG_ERROR, LOG_DEBUG, LOG_INFO)
#endif

#include <vdr/timers.h>

namespace vdrlive {

	class SortedTimers: public std::list<cTimer>
	{
		friend class TimerManager;

		public:
			static std::string GetTimerId(cTimer const& timer);
			const cTimer* GetByTimerId(std::string const& timerid);

			// en- or decodes a timer into an id usable for DOM Ids.
			static std::string EncodeDomId(std::string const& timerid);
			static std::string DecodeDomId(std::string const &timerDomId);

#if VDRVERSNUM >= 20301
			bool Modified();
#else
			bool Modified() { return Timers.Modified(m_state); }
#endif

			static std::string GetTimerDays(cTimer const *timer);
			static std::string GetTimerInfo(cTimer const& timer);
			static std::string SearchTimerInfo(cTimer const& timer, std::string const& value);

		private:
			SortedTimers();
			SortedTimers( SortedTimers const& );

			cMutex m_mutex;

#if VDRVERSNUM >= 20301
			cStateKey m_TimersStateKey;
#else
			int m_state;
#endif

	};

	class TimerManager: public cMutex
	{
		friend TimerManager& LiveTimerManager();

		public:
			SortedTimers& GetTimers() { return m_timers; }

			void UpdateTimer( int timerId, const char* remote, const char* oldRemote, int flags, const tChannelID& channel, std::string const& weekdays,
					  std::string const& day, int start, int stop, int priority, int lifetime, std::string const& title, std::string const& aux );

			void DelTimer( int timerId, const char* remote);
			void ToggleTimerActive( int timerId, const char* remote);
			// may only be called from Plugin::MainThreadHook
			void DoPendingWork();
			const cTimer* GetTimer(tEventID eventid, tChannelID channelid);
			void SetReloadTimers() { m_reloadTimers = true; }

		private:
			typedef struct
                        {
                          int id;
                          const char* remote;
                          const char* oldRemote;
			  std::string builder;
                        } timerStruct;

			typedef std::pair<timerStruct, std::string> ErrorPair;
			typedef std::list<timerStruct> TimerList;
			typedef std::list<ErrorPair> ErrorList;

			TimerManager();
			TimerManager( TimerManager const& );

			SortedTimers m_timers;
			TimerList m_updateTimers;
			ErrorList m_failedUpdates;
			cCondVar m_updateWait;
			bool m_reloadTimers = false;

			void DoUpdateTimers();
			void DoInsertTimer( timerStruct& timerData );
			void DoUpdateTimer( timerStruct& timerData );
			void DoDeleteTimer( timerStruct& timerData );
			void DoToggleTimer( timerStruct& timerData );

			void StoreError( timerStruct const& timerData, std::string const& error );
			std::string GetError( timerStruct const& timerData );
	};

	TimerManager& LiveTimerManager();

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
