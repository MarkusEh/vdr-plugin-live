#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

// STL headers need to be before VDR tools.h (included by <vdr/timers.h>)
#include <list>
#include <string>

#include <vdr/timers.h>

namespace vdrlive {

	class SortedTimers: public std::list< cTimer >
	{
		friend class TimerManager;

		public:
			static std::string GetTimerId(cTimer const& timer);
			cTimer* GetByTimerId(std::string const& timerid);

			// en- or decodes a timer into an id usable for DOM Ids.
			static std::string EncodeDomId(std::string const& timerid);
			static std::string DecodeDomId(std::string const &timerDomId);

#if VDRVERSNUM >= 20301
			bool Modified();
#else
			bool Modified() { return Timers.Modified(m_state); }
#endif

			static std::string GetTimerDays(cTimer const& timer);
			static std::string GetTimerInfo(cTimer const& timer);
			static std::string SearchTimerName(cTimer const& timer);

		private:
			SortedTimers();
			SortedTimers( SortedTimers const& );

			int m_state;

			void ReloadTimers( bool initial = false );
	};

	class TimerManager: public cMutex
	{
		friend TimerManager& LiveTimerManager();

		public:
			SortedTimers& GetTimers() { return m_timers; }

			void UpdateTimer( cTimer* timer, int flags, tChannelID& channel, std::string const& weekdays, std::string const& day,
							  int start, int stop, int priority, int lifetime, std::string const& title, std::string const& aux );

			void DelTimer( cTimer* timer);
			void ToggleTimerActive( cTimer* timer);
			// may only be called from Plugin::MainThreadHook
			void DoPendingWork();
			void DoReloadTimers() { m_timers.ReloadTimers(); m_reloadTimers = false; }
			const cTimer* GetTimer(tEventID eventid, tChannelID channelid);
			void SetReloadTimers() { m_reloadTimers = true; }

		private:
			typedef std::pair< cTimer*, std::string > TimerPair;
			typedef std::pair< TimerPair, std::string > ErrorPair;
			typedef std::list< TimerPair > TimerList;
			typedef std::list< ErrorPair > ErrorList;

			TimerManager();
			TimerManager( TimerManager const& );

			SortedTimers m_timers;
			TimerList m_updateTimers;
			ErrorList m_failedUpdates;
			cCondVar m_updateWait;
			bool m_reloadTimers;

			void DoUpdateTimers();
			void DoInsertTimer( TimerPair& timerData );
			void DoUpdateTimer( TimerPair& timerData );
			void DoDeleteTimer( TimerPair& timerData );
			void DoToggleTimer( TimerPair& timerData );

			void StoreError( TimerPair const& timerData, std::string const& error );
			std::string GetError( TimerPair const& timerData );
	};

	TimerManager& LiveTimerManager();

} // namespace vdrlive

#endif // VDR_LIVE_TIMERS_H
