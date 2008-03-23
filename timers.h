#ifndef VDR_LIVE_TIMERS_H
#define VDR_LIVE_TIMERS_H

#include <list>
#include <string>
#include <vdr/channels.h>
#include <vdr/menu.h>
#include <vdr/timers.h>
#include <vdr/thread.h>
#include "live.h"

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

			bool Modified() { return Timers.Modified(m_state); }

			static std::string GetTimerDays(cTimer const& timer);
			static std::string GetTimerInfo(cTimer const& timer);

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
			void DoReloadTimers() { m_timers.ReloadTimers(); }
			const cTimer* GetTimer(tEventID eventid, tChannelID channelid);

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
